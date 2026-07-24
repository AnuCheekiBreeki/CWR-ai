#!/usr/bin/env python3
"""Scan Remaster/OFP game data for CfgWeapons classes and emit an arsenal catalog.

Reads text config.cpp and binary raP config.bin from AddOns/*.pbo and BIN/CONFIG.BIN.
"""
from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from dataclasses import dataclass, field
from pathlib import Path


# --- PBO -----------------------------------------------------------------


def read_pbo(path: Path) -> dict[str, bytes]:
    data = path.read_bytes()
    i = 0
    entries: list[tuple[str, int, int]] = []
    # Skip optional Vers properties header
    if len(data) > 21 and data[0] == 0 and data[1:5] == b"sreV"[::-1]:
        # packing = Vers little-endian 0x56657273
        pass
    while True:
        end = data.find(b"\x00", i)
        if end < 0:
            break
        name = data[i:end].decode("latin1")
        i = end + 1
        if i + 20 > len(data):
            break
        packing, original, reserved, timestamp, size = struct.unpack_from("<5I", data, i)
        i += 20
        if name == "" and packing == 0x56657273:
            # properties block until empty key
            while True:
                kend = data.find(b"\x00", i)
                key = data[i:kend].decode("latin1")
                i = kend + 1
                if key == "":
                    break
                vend = data.find(b"\x00", i)
                i = vend + 1
            continue
        entries.append((name, packing, size))
        if name == "":
            break
    files: dict[str, bytes] = {}
    for name, packing, size in entries:
        if not name:
            continue
        chunk = data[i : i + size]
        i += size
        if packing == 0:
            files[name.replace("\\", "/")] = chunk
    return files


# --- raP parser ----------------------------------------------------------


class RapError(Exception):
    pass


class RapReader:
    def __init__(self, data: bytes):
        self.data = data
        self.pos = 0
        self.strings: list[str] = []
        self.version = 0

    def rest(self) -> int:
        return len(self.data) - self.pos

    def get(self) -> int:
        if self.pos >= len(self.data):
            raise RapError("EOF")
        b = self.data[self.pos]
        self.pos += 1
        return b

    def read(self, n: int) -> bytes:
        if self.pos + n > len(self.data):
            raise RapError("EOF")
        out = self.data[self.pos : self.pos + n]
        self.pos += n
        return out

    def read_u32(self) -> int:
        return struct.unpack("<I", self.read(4))[0]

    def read_f32(self) -> float:
        return struct.unpack("<f", self.read(4))[0]

    def read_i32(self) -> int:
        return struct.unpack("<i", self.read(4))[0]

    def read_cstring(self) -> str:
        buf = bytearray()
        while True:
            c = self.get()
            if c == 0:
                break
            buf.append(c)
        return buf.decode("latin1", "replace")

    def read_varint(self) -> int:
        val = 0
        offset = 0
        while True:
            if self.rest() <= 0 or offset >= 32:
                raise RapError("bad varint")
            c = self.get()
            val |= (c & 0x7F) << offset
            if (c & 0x80) == 0:
                break
            offset += 7
        return val

    def transfer_index(self) -> int:
        if self.version >= 3:  # verEncode default 3 for strings; class count uses 4
            return self.read_varint()
        return self.read_i32()

    def transfer_index_n(self, ver_encode: int) -> int:
        if self.version >= ver_encode:
            return self.read_varint()
        return self.read_i32()

    def transfer_string(self) -> str:
        index = self.transfer_index()
        if index < 0:
            raise RapError("neg string index")
        if index < len(self.strings):
            return self.strings[index]
        if index != len(self.strings):
            raise RapError(f"string index gap {index} vs {len(self.strings)}")
        s = self.read_cstring()
        self.strings.append(s)
        return s


def parse_rap(data: bytes) -> dict:
    """Return nested dict: classes -> dict, values -> scalar/list."""
    if len(data) < 8 or data[0:4] not in (b"\x00raP", b"Par\x00"):
        raise RapError("not raP")
    r = RapReader(data)
    r.pos = 4
    r.version = r.read_u32()

    def parse_array_value():
        t = r.get()
        if t == 0:  # SVGeneric string
            return r.transfer_string()
        if t == 1:  # float
            return r.read_f32()
        if t == 2:  # int
            return r.read_i32()
        if t == 3:  # nested array
            n = r.transfer_index_n(4)
            return [parse_array_value() for _ in range(n)]
        raise RapError(f"bad array value type {t}")

    def parse_class() -> dict:
        name = r.transfer_string()
        base = r.read_cstring()  # Transfer(RStringB) — NOT string-table for base!
        # Wait - looking at code: f.Transfer(base) where base is RStringB uses operator<<
        # which is cstring, NOT TransferString. Yes.
        n = r.transfer_index_n(4)
        entries: dict = {"__name__": name, "__base__": base or None}
        for _ in range(n):
            eid = r.get()
            if eid == 0:  # class
                child = parse_class()
                entries[child["__name__"]] = child
            elif eid == 2:  # array
                aname = r.transfer_string()
                count = r.transfer_index_n(4)
                entries[aname] = [parse_array_value() for _ in range(count)]
            else:  # value (id 1)
                vtype = r.get()
                vname = r.transfer_string()
                if vtype == 0:
                    entries[vname] = r.transfer_string()
                elif vtype == 1:
                    entries[vname] = r.read_f32()
                elif vtype == 2:
                    entries[vname] = r.read_i32()
                else:
                    entries[vname] = r.transfer_string()
        return entries

    root = parse_class()
    return root


# --- text config ---------------------------------------------------------


CLASS_RE = re.compile(
    r"class\s+([A-Za-z_][\w]*)\s*(?::\s*([A-Za-z_][\w]*))?\s*\{",
    re.MULTILINE,
)


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//.*?$", "", text, flags=re.M)
    return text


def extract_cfgweapons_text(text: str) -> list[dict]:
    text = strip_comments(text)
    # Find CfgWeapons block by brace matching
    m = re.search(r"class\s+CfgWeapons\b", text)
    if not m:
        return []
    start = text.find("{", m.end())
    if start < 0:
        return []
    depth = 0
    i = start
    while i < len(text):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                break
        i += 1
    body = text[start + 1 : i]
    items = []
    # Top-level classes inside CfgWeapons (brace depth 1 relative to body)
    pos = 0
    while True:
        m = CLASS_RE.search(body, pos)
        if not m:
            break
        # ensure this class is at depth 0 in body
        before = body[: m.start()]
        if before.count("{") - before.count("}") != 0:
            pos = m.end()
            continue
        cname, base = m.group(1), m.group(2)
        bstart = body.find("{", m.end() - 1)
        depth = 0
        j = bstart
        while j < len(body):
            if body[j] == "{":
                depth += 1
            elif body[j] == "}":
                depth -= 1
                if depth == 0:
                    break
            j += 1
        cbody = body[bstart + 1 : j]
        def num(key, default=None):
            mm = re.search(rf"\b{key}\s*=\s*([^;]+);", cbody)
            if not mm:
                return default
            raw = mm.group(1).strip()
            raw = re.sub(r"/\*.*?\*/", "", raw)
            # resolve simple defines
            for name, val in (
                ("public", 2),
                ("protected", 1),
                ("private", 0),
                ("true", 1),
                ("false", 0),
                ("WeaponSlotPrimary", 1),
                ("WeaponSlotSecondary", 16),
                ("WeaponSlotItem", 256),
                ("WeaponSlotBinocular", 4096),
                ("WeaponSlotHandGun", 2),
                ("WeaponSlotHandGunItem", 16),  # approximate; remaster uses 0xE0 mask
            ):
                if raw == name:
                    return val
            try:
                if raw.lower().startswith("0x"):
                    return int(raw, 16)
                if re.fullmatch(r"-?\d+", raw):
                    return int(raw)
                return float(raw)
            except ValueError:
                return raw.strip('"')

        def arr(key):
            mm = re.search(r"\b" + re.escape(key) + r"\s*\[\]\s*=\s*\{([^}]*)\}", cbody)
            if not mm:
                return []
            return [x.strip().strip('"') for x in mm.group(1).split(",") if x.strip()]

        items.append(
            {
                "class": cname,
                "base": base,
                "scopeWeapon": num("scopeWeapon", num("scope")),
                "scopeMagazine": num("scopeMagazine"),
                "weaponType": num("weaponType", num("type")),
                "magazineType": num("magazineType"),
                "displayName": num("displayName", cname),
                "magazines": arr("magazines"),
                "model": num("model"),
                "picture": num("picture"),
            }
        )
        pos = j + 1
    return items


def walk_rap_cfgweapons(node: dict, out: list[dict], path: list[str] | None = None) -> None:
    path = path or []
    name = node.get("__name__", "")
    # Find CfgWeapons
    if name == "CfgWeapons" or (path and path[-1] == "CfgWeapons"):
        for k, v in node.items():
            if k.startswith("__") or not isinstance(v, dict):
                continue
            out.append(
                {
                    "class": k,
                    "base": v.get("__base__"),
                    "scopeWeapon": v.get("scopeWeapon", v.get("scope")),
                    "scopeMagazine": v.get("scopeMagazine"),
                    "weaponType": v.get("weaponType", v.get("type")),
                    "magazineType": v.get("magazineType"),
                    "displayName": v.get("displayName", k),
                    "magazines": v.get("magazines") if isinstance(v.get("magazines"), list) else [],
                    "model": v.get("model"),
                    "picture": v.get("picture"),
                }
            )
        return
    for k, v in node.items():
        if isinstance(v, dict) and not k.startswith("__"):
            walk_rap_cfgweapons(v, out, path + [k])


# --- catalog -------------------------------------------------------------

SLOT_PRIMARY = 0x00000001
SLOT_HANDGUN = 0x00000002
SLOT_SECONDARY = 0x00000010
SLOT_ITEM = 0x00000F00
SLOT_BINOCULAR = 0x00003000
SLOT_HANDGUN_ITEM = 0x000000E0


def _as_int(val, default=None):
    if val is None or val == "":
        return default
    if isinstance(val, bool):
        return int(val)
    if isinstance(val, (int, float)):
        return int(val)
    if isinstance(val, str):
        s = val.strip()
        # Evaluate simple OFP expressions like "2 * 256"
        s = s.replace("\t", " ")
        for name, num in (
            ("public", "2"),
            ("protected", "1"),
            ("private", "0"),
            ("WeaponSlotPrimary", "1"),
            ("WeaponSlotHandGun", "2"),
            ("WeaponSlotSecondary", "16"),
            ("WeaponSlotItem", "256"),
            ("WeaponSlotBinocular", "4096"),
            ("WeaponHardMounted", "65536"),
            ("true", "1"),
            ("false", "0"),
        ):
            s = re.sub(rf"\b{name}\b", num, s)
        if re.fullmatch(r"-?\d+", s):
            return int(s)
        if re.fullmatch(r"0x[0-9A-Fa-f]+", s):
            return int(s, 16)
        if re.fullmatch(r"[\d\s\*\+\-]+", s):
            try:
                return int(eval(s, {"__builtins__": {}}, {}))  # noqa: S307 - curated arithmetic only
            except Exception:
                return default
    return default


def resolve_inheritance(items: list[dict]) -> list[dict]:
    # Merge duplicate class definitions (addons override base), keeping any known fields.
    by_name: dict[str, dict] = {}
    for it in items:
        name = it["class"]
        if name not in by_name:
            by_name[name] = dict(it)
            continue
        cur = by_name[name]
        for k, v in it.items():
            if v is None or v == "" or v == []:
                continue
            if cur.get(k) in (None, "", []):
                cur[k] = v
            elif k == "source":
                cur[k] = f"{cur.get(k)};{v}"
            else:
                # Prefer later (addon) non-empty values for most fields
                cur[k] = v
        if it.get("base") and not cur.get("base"):
            cur["base"] = it["base"]

    def lookup(name: str, field: str, seen: set[str] | None = None):
        if not name or name not in by_name:
            return None
        seen = seen or set()
        if name in seen:
            return None
        seen.add(name)
        node = by_name[name]
        val = node.get(field)
        if val is not None and val != "" and val != []:
            return val
        base = node.get("base")
        if base:
            return lookup(base, field, seen)
        return None

    out = []
    for name, node in by_name.items():
        merged = dict(node)
        for field in (
            "scopeWeapon",
            "scopeMagazine",
            "weaponType",
            "magazineType",
            "displayName",
            "model",
            "picture",
            "magazines",
        ):
            if merged.get(field) in (None, "", []):
                inherited = lookup(name, field)
                if inherited is not None:
                    merged[field] = inherited
        out.append(merged)
    return out


def categories_for(item: dict) -> list[str]:
    """OFP classes can be both a weapon and a magazine (same classname)."""
    sw = _as_int(item.get("scopeWeapon"))
    sm = _as_int(item.get("scopeMagazine"))
    wt = _as_int(item.get("weaponType"), 0) or 0
    mt = _as_int(item.get("magazineType"), 0) or 0
    cats: list[str] = []

    if sw is not None and sw >= 2:
        if wt & 0x00010000 and not (wt & (SLOT_PRIMARY | SLOT_SECONDARY | SLOT_HANDGUN | SLOT_BINOCULAR)):
            pass  # vehicle hard-mounted
        elif wt & SLOT_SECONDARY and not (wt & SLOT_PRIMARY):
            cats.append("secondary")
        elif wt & SLOT_HANDGUN:
            cats.append("handgun")
        elif wt & SLOT_BINOCULAR:
            cats.append("binocular")
        elif wt & SLOT_PRIMARY:
            cats.append("primary")
        elif wt == 0:
            # Missing type but public weapon — assume primary (rifles)
            cats.append("primary")

    if sm is not None and sm >= 2:
        is_weapon = bool(wt & (SLOT_PRIMARY | SLOT_SECONDARY | SLOT_HANDGUN | SLOT_BINOCULAR)) or (
            sw is not None and sw >= 2 and wt == 0
        )
        if mt & SLOT_HANDGUN_ITEM:
            cats.append("handgun_mag")
        elif (mt & SLOT_ITEM) and not is_weapon:
            cats.append("item")
        elif not is_weapon or (wt & SLOT_SECONDARY) or item["class"].endswith("Mag"):
            # Dedicated magazines, launcher ammo (often same class), *Mag classes
            cats.append("magazine")
        elif is_weapon and (wt & SLOT_PRIMARY or wt == 0):
            # OFP combined rifle/mag classname (addMagazine "AK74")
            cats.append("magazine")

    # De-dupe preserving order
    seen = set()
    out = []
    for c in cats:
        if c not in seen:
            seen.add(c)
            out.append(c)
    return out


def scan_game(root: Path) -> dict[str, list[dict]]:
    raw_all: list[dict] = []

    def collect(items: list[dict], source: str):
        for it in items:
            it = dict(it)
            it["source"] = source
            raw_all.append(it)

    # BIN/CONFIG.BIN
    cfg = root / "BIN" / "CONFIG.BIN"
    if not cfg.exists():
        cfg = root / "bin" / "config.bin"
    if cfg.exists():
        try:
            tree = parse_rap(cfg.read_bytes())
            items: list[dict] = []
            if "CfgWeapons" in tree and isinstance(tree["CfgWeapons"], dict):
                cw = {"__name__": "CfgWeapons", **tree["CfgWeapons"]}
                walk_rap_cfgweapons(cw, items)
            else:
                walk_rap_cfgweapons(tree, items)
            collect(items, str(cfg))
            print(f"CONFIG.BIN: {len(items)} raw CfgWeapons classes", file=sys.stderr)
        except Exception as e:
            print(f"warn: failed to parse {cfg}: {e}", file=sys.stderr)

    addons = root / "AddOns"
    if not addons.exists():
        addons = root / "addons"
    if addons.exists():
        for pbo in sorted(addons.glob("*.[pP][bB][oO]")):
            try:
                files = read_pbo(pbo)
            except Exception as e:
                print(f"warn: pbo {pbo.name}: {e}", file=sys.stderr)
                continue
            for name, blob in files.items():
                low = name.lower()
                if low.endswith("config.cpp"):
                    collect(extract_cfgweapons_text(blob.decode("latin1", "replace")), f"{pbo.name}:{name}")
                elif "config.bin" in low or low.endswith(".bin"):
                    if blob[:4] not in (b"\x00raP", b"Par\x00"):
                        continue
                    try:
                        tree = parse_rap(blob)
                        items = []
                        if "CfgWeapons" in tree and isinstance(tree["CfgWeapons"], dict):
                            cw = {"__name__": "CfgWeapons", **tree["CfgWeapons"]}
                            walk_rap_cfgweapons(cw, items)
                        else:
                            walk_rap_cfgweapons(tree, items)
                        collect(items, f"{pbo.name}:{name}")
                    except Exception as e:
                        print(f"warn: rap {pbo.name}/{name}: {e}", file=sys.stderr)

    merged = resolve_inheritance(raw_all)
    # One entry may belong to multiple arsenal tabs (OFP combined weapon/mag classes).
    multi: dict[str, list[dict]] = {
        "primary": [],
        "secondary": [],
        "handgun": [],
        "binocular": [],
        "magazine": [],
        "handgun_mag": [],
        "item": [],
    }
    skip_bases = {
        "Default",
        "MGun",
        "Riffle",
        "Rifle",
        "BulletSingle",
        "Throw",
        "Put",
        "HandGunBase",
        "G36aBase",
        "BizonBase",
        "SteyrBase",
    }
    for it in sorted(merged, key=lambda x: x["class"].lower()):
        if it["class"] in skip_bases:
            continue
        cats = categories_for(it)
        for cat in cats:
            multi[cat].append({**it, "category": cat})
    return multi


def emit_sqs_catalog(categories: dict[str, list[dict]], out: Path) -> None:
    """Emit a SQS helper that fills global arrays used by the arsenal dialog."""
    lines = [
        "; Auto-generated by mods/tools/scan_weapons.py - do not edit by hand",
        "VA_Primary = []",
        "VA_Secondary = []",
        "VA_Handgun = []",
        "VA_Binocular = []",
        "VA_Magazine = []",
        "VA_HandgunMag = []",
        "VA_Item = []",
        "",
    ]

    def emit(var: str, items: list[dict]):
        for it in items:
            cls = it["class"]
            # skip Throw/Put internals if present
            if cls in ("Throw", "Put", "Default", "MGun", "Riffle", "Rifle", "BulletSingle"):
                continue
            lines.append(f'{var} = {var} + ["{cls}"]')

    emit("VA_Primary", categories["primary"])
    emit("VA_Secondary", categories["secondary"])
    emit("VA_Handgun", categories["handgun"])
    emit("VA_Binocular", categories["binocular"])
    emit("VA_Magazine", categories["magazine"] + categories["handgun_mag"])
    emit("VA_Item", categories["item"])
    lines.append('hint format ["VA catalog: %1 primaries", count VA_Primary]')
    out.write_text("\n".join(lines) + "\n", encoding="latin1")
    print(f"wrote {out}")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("gamedir", type=Path, help="Game data root (contains BIN/, AddOns/)")
    ap.add_argument("-o", "--output", type=Path, required=True, help="Output catalog.sqs path")
    ap.add_argument("--json", type=Path, help="Also write JSON dump")
    args = ap.parse_args()
    cats = scan_game(args.gamedir)
    for k, v in cats.items():
        print(f"  {k}: {len(v)}")
    emit_sqs_catalog(cats, args.output)
    if args.json:
        args.json.write_text(json.dumps(cats, indent=2, default=str), encoding="utf-8")
        print(f"wrote {args.json}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
