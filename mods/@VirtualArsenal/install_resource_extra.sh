#!/usr/bin/env bash
# Merge Virtual Arsenal dialog classes into the game's BIN/resource-extra.cpp
# so createDialog works outside the demo mission. Safe to re-run (idempotent).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FRAG="$ROOT/bin/resource_arsenal.hpp"
MARKER_BEGIN="// BEGIN VirtualArsenal"
MARKER_END="// END VirtualArsenal"

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
[[ -f "$FRAG" ]] || { echo "error: missing $FRAG"; exit 1; }

EXTRA=""
for p in "$GAME_DIR/BIN/resource-extra.cpp" "$GAME_DIR/bin/resource-extra.cpp"; do
  if [[ -f "$p" ]]; then EXTRA="$p"; break; fi
done
[[ -n "$EXTRA" ]] || { echo "error: resource-extra.cpp not found under $GAME_DIR"; exit 1; }

cp -a "$EXTRA" "$EXTRA.bak.$(date +%Y%m%d%H%M%S)"

python3 - <<PY
from pathlib import Path
extra = Path("""$EXTRA""")
frag = Path("""$FRAG""")
begin = """$MARKER_BEGIN"""
end = """$MARKER_END"""
text = extra.read_text(encoding="latin1", errors="replace")
if begin in text and end in text:
    pre, rest = text.split(begin, 1)
    _, post = rest.split(end, 1)
    text = pre.rstrip() + "\n" + post.lstrip("\n")
block = begin + "\n" + frag.read_text(encoding="latin1", errors="replace") + "\n" + end + "\n"
extra.write_text(text.rstrip() + "\n\n" + block, encoding="latin1")
print(f"updated {extra}")
PY

echo "Dialog classes installed into $EXTRA"
echo "Restart the game; Virtual Arsenal UserAction works in any mission."
