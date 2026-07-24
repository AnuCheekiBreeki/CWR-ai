# AIO Command Menu (`@AIOCommandMenu`)

Poseidon / Cold War Assault Remastered MVP inspired by
[All-In-One Command Menu](https://github.com/Leopard20/All-In-One-Command-Menu).

Arma 3 AIO relies on CBA, `showCommandingMenu`, Zeus, and ACE — none of that
exists here. This mod is a Poseidon-native hub:

- **Command menu** dialog (list + Execute)
- **Virtual Arsenal** (embedded, same catalog/camera flow as `@VirtualArsenal`)
- **Cheats**: heal, god mode, refill mags, map teleport, skip time, accTime, captive

## Features

| Menu entry | Action |
|---|---|
| Virtual Arsenal | Gear browser (primary / launcher / handgun / optics / mags / items) |
| Heal | `setDamage 0` |
| God mode | toggle `allowDamage` |
| Refill magazines | top up for equipped weapons |
| Teleport | `forceMap` + `onMapSingleClick` |
| Skip time | `skipTime 1` |
| AccTime | cycle 1x → 2x → 4x |
| Captive | toggle `setCaptive` |

Scroll-menu UserActions on the player: **AIO Command Menu** and **Virtual Arsenal**.

## Important: dialogs and `bin/resource.cpp`

Do **not** put a full `bin/resource.cpp` in the mod. Poseidon stops resource
enumeration at the first mod that ships one and replaces `RESOURCE.BIN`, which
breaks the remaster UI. Dialog classes are delivered via:

1. Demo mission `Missions/AIO_Demo.Abel/description.ext` (works immediately), or
2. `./install_resource_extra.sh` — merges arsenal + menu headers into the game's
   `BIN/resource-extra.cpp` so dialogs are global.

## Pack

```sh
export CWR_GAME_DIR="$HOME/cwr-game/Remaster"   # folder with BIN/, AddOns/, …
./mods/@AIOCommandMenu/pack.sh
./mods/@AIOCommandMenu/install_resource_extra.sh  # optional, for global dialogs
```

## Run

```sh
./PoseidonGame \
  -C "$CWR_GAME_DIR" \
  --mods-dir /path/to/repo/mods \
  --mod @AIOCommandMenu \
  --window -w 1280 -h 720 \
  "Missions/AIO_Demo.Abel"
```

Or copy `Missions/AIO_Demo.Abel` into the game `Missions/` folder. In-game:
action menu → **AIO Command Menu**.

## Layout

```
mods/@AIOCommandMenu/
  mod.json
  pack.sh
  install_resource_extra.sh
  bin/resource_arsenal.hpp
  bin/resource_aio.hpp
  addons/aio.pbo
  addons/aio_src/
    config.cpp
    scripts/open_menu.sqs
    scripts/menu_select.sqs
    scripts/arsenal/
    scripts/cheats/
  Missions/AIO_Demo.Abel/
mods/tools/pack_pbo.py
mods/tools/scan_weapons.py
```

## Notes

- PBO prefix is `aio` → scripts load as `\aio\scripts\...`
- Weapon catalog is generated offline (`scan_weapons.py`); re-run `pack.sh` after
  adding weapon mods.
- After PR merge of `@VirtualArsenal` onto this branch, you can load either mod;
  AIO stays self-contained for the MVP.
