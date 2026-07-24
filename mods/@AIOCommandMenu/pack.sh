#!/usr/bin/env bash
# Pack @AIOCommandMenu into addons/aio.pbo and refresh the weapon catalog.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MOD="$ROOT/@AIOCommandMenu"
SRC="$MOD/addons/aio_src"
OUT="$MOD/addons/aio.pbo"
TOOLS="$ROOT/tools"

GAME_DIR="${CWR_GAME_DIR:-${POSEIDON_GAME_DIR:-}}"
if [[ -z "$GAME_DIR" ]]; then
  for candidate in \
      "$ROOT/../packages/Remaster" \
      "$HOME/cwr-game/Remaster" \
      "$HOME/cwr-assets/Remastered/Remastered"
  do
    if [[ -d "$candidate/BIN" || -d "$candidate/bin" ]]; then
      GAME_DIR="$candidate"
      break
    fi
  done
fi

if [[ -n "$GAME_DIR" && -d "$GAME_DIR" ]]; then
  echo "==> scanning weapons from $GAME_DIR"
  python3 "$TOOLS/scan_weapons.py" "$GAME_DIR" \
      -o "$SRC/scripts/arsenal/catalog.sqs" \
      --json "$MOD/catalog.json"
else
  echo "warning: no game data dir found; keeping existing catalog.sqs" >&2
  echo "         set CWR_GAME_DIR to Remaster root (with BIN/, AddOns/)" >&2
fi

echo "==> packing $SRC -> $OUT"
python3 "$TOOLS/pack_pbo.py" "$SRC" "$OUT" --prefix aio

echo "==> done"
echo "    Load with:  --mod $MOD"
ls -la "$OUT"
