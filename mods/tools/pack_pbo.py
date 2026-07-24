#!/usr/bin/env python3
"""Pack a directory into an OFP/CWA-style uncompressed PBO (PoseidonTools-compatible)."""
from __future__ import annotations

import argparse
import os
import struct
import sys
import time
from pathlib import Path


def _iter_files(src: Path):
    for root, _dirs, files in os.walk(src):
        for name in files:
            full = Path(root) / name
            rel = full.relative_to(src).as_posix()
            yield rel, full


def pack_pbo(src: Path, dst: Path, prefix: str | None = None) -> None:
    src = src.resolve()
    if not src.is_dir():
        raise SystemExit(f"source is not a directory: {src}")

    entries: list[tuple[str, bytes, int]] = []
    for rel, full in sorted(_iter_files(src), key=lambda x: x[0].lower()):
        data = full.read_bytes()
        mtime = int(full.stat().st_mtime)
        entries.append((rel.replace("/", "\\"), data, mtime))

    dst.parent.mkdir(parents=True, exist_ok=True)
    with dst.open("wb") as out:
        # Optional product header properties (prefix) for bank path resolution.
        if prefix:
            # Empty-name entry with packing method "Vers" (0x56657273) introduces properties.
            out.write(b"\x00")
            out.write(struct.pack("<I", 0x56657273))  # "Vers"
            out.write(struct.pack("<I", 0))
            out.write(struct.pack("<I", 0))
            out.write(struct.pack("<I", 0))
            out.write(struct.pack("<I", 0))
            # property: prefix=<prefix>\0 then empty key terminator
            out.write(b"prefix\x00")
            out.write(prefix.encode("latin1") + b"\x00")
            out.write(b"\x00")

        for name, data, mtime in entries:
            out.write(name.encode("latin1") + b"\x00")
            out.write(struct.pack("<I", 0))  # packing
            out.write(struct.pack("<I", 0))  # original size (unused when uncompressed)
            out.write(struct.pack("<I", 0))  # reserved
            out.write(struct.pack("<I", mtime))
            out.write(struct.pack("<I", len(data)))

        # End of header
        out.write(b"\x00")
        out.write(struct.pack("<5I", 0, 0, 0, 0, 0))

        for _name, data, _mtime in entries:
            out.write(data)

    print(f"packed {len(entries)} files -> {dst} ({dst.stat().st_size} bytes)")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("source", type=Path, help="Source directory to pack")
    ap.add_argument("output", type=Path, help="Output .pbo path")
    ap.add_argument(
        "--prefix",
        default=None,
        help="Optional PBO prefix (bank root name used by \\prefix\\... paths)",
    )
    args = ap.parse_args()
    pack_pbo(args.source, args.output, args.prefix)
    return 0


if __name__ == "__main__":
    sys.exit(main())
