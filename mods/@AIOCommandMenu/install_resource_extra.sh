#!/usr/bin/env bash
# Merge AIO Command Menu + Virtual Arsenal dialogs into BIN/resource-extra.cpp.
# Safe to re-run (idempotent). Do NOT ship bin/resource.cpp in the mod.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ARSENAL="$ROOT/bin/resource_arsenal.hpp"
MENU="$ROOT/bin/resource_aio.hpp"
MARKER_BEGIN="// BEGIN AIOCommandMenu"
MARKER_END="// END AIOCommandMenu"
# Also strip a prior @VirtualArsenal-only install so classes are not duplicated.
VA_BEGIN="// BEGIN VirtualArsenal"
VA_END="// END VirtualArsenal"

GAME_DIR="${CWR_GAME_DIR:-${POSEIDON_GAME_DIR:-}}"
if [[ -z "$GAME_DIR" ]]; then
  for candidate in \
      "$ROOT/../../packages/Remaster" \
      "$HOME/cwr-game/Remaster" \
      "$HOME/cwr-assets/Remastered/Remastered"
  do
    if [[ -f "$candidate/BIN/resource-extra.cpp" || -f "$candidate/bin/resource-extra.cpp" ]]; then
      GAME_DIR="$candidate"
      break
    fi
  done
fi

[[ -n "$GAME_DIR" ]] || { echo "error: set CWR_GAME_DIR"; exit 1; }
[[ -f "$ARSENAL" ]] || { echo "error: missing $ARSENAL"; exit 1; }
[[ -f "$MENU" ]] || { echo "error: missing $MENU"; exit 1; }

EXTRA=""
for p in "$GAME_DIR/BIN/resource-extra.cpp" "$GAME_DIR/bin/resource-extra.cpp"; do
  if [[ -f "$p" ]]; then EXTRA="$p"; break; fi
done
[[ -n "$EXTRA" ]] || { echo "error: resource-extra.cpp not found under $GAME_DIR"; exit 1; }

cp -a "$EXTRA" "$EXTRA.bak.$(date +%Y%m%d%H%M%S)"

python3 - <<PY
from pathlib import Path
extra = Path("""$EXTRA""")
arsenal = Path("""$ARSENAL""")
menu = Path("""$MENU""")
begin = """$MARKER_BEGIN"""
end = """$MARKER_END"""
va_begin = """$VA_BEGIN"""
va_end = """$VA_END"""

def strip_block(text: str, b: str, e: str) -> str:
    if b in text and e in text:
        pre, rest = text.split(b, 1)
        _, post = rest.split(e, 1)
        return pre.rstrip() + "\n" + post.lstrip("\n")
    return text

text = extra.read_text(encoding="latin1", errors="replace")
text = strip_block(text, begin, end)
text = strip_block(text, va_begin, va_end)
# Flatten: arsenal first, then menu without its #include line
menu_body = []
for line in menu.read_text(encoding="latin1", errors="replace").splitlines():
    if line.strip().startswith("#include"):
        continue
    menu_body.append(line)
frag = arsenal.read_text(encoding="latin1", errors="replace").rstrip() + "\n\n" + "\n".join(menu_body).rstrip() + "\n"
block = begin + "\n" + frag + end + "\n"
extra.write_text(text.rstrip() + "\n\n" + block, encoding="latin1")
print(f"updated {extra}")
PY

echo "Dialog classes installed into $EXTRA"
echo "Restart the game; AIO UserActions work in any mission."
