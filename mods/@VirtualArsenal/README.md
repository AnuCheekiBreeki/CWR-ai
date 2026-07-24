# Virtual Arsenal (`@VirtualArsenal`)

OFP/CWA Remastered gear browser inspired by Arma 3 Virtual Arsenal / ACE Arsenal.

## Features

- Scroll-menu **Virtual Arsenal** action on the player (`CfgVehicles` / `UserActions`)
- Tabs: **Primary / Launcher / Handgun / Optics / Mags / Items**
- Catalog scanned from `BIN/CONFIG.BIN` + `AddOns/*.pbo` `CfgWeapons`
- Apply / clear gear
- Orbit camera preview around the soldier

## Important: dialogs and `bin/resource.cpp`

Do **not** put a full `bin/resource.cpp` in the mod. Poseidon stops resource
enumeration at the first mod that ships one and replaces `RESOURCE.BIN`, which
breaks the remaster UI. Dialog classes are delivered via:

1. Demo mission `Missions/VirtualArsenal.Intro/description.ext` (works immediately), or
2. `./install_resource_extra.sh` — merges `bin/resource_arsenal.hpp` into the game's
   `BIN/resource-extra.cpp` so the dialog is global.

## Pack

```sh
export CWR_GAME_DIR="$HOME/cwr-game/Remaster"   # folder with BIN/, AddOns/, …
./mods/@VirtualArsenal/pack.sh
./mods/@VirtualArsenal/install_resource_extra.sh  # optional, for global dialog
```

## Run

```sh
./PoseidonGame \
  -C "$CWR_GAME_DIR" \
  --mods-dir /path/to/repo/mods \
  --mod @VirtualArsenal \
  --window -w 1280 -h 720 \
  "Missions/VirtualArsenal.Intro"
```

Or copy `Missions/VirtualArsenal.Intro` into the game `Missions/` folder and pick it
from the singleplayer list. In-game: action menu → **Virtual Arsenal**.

## Layout

```
mods/@VirtualArsenal/
  mod.json
  pack.sh
  install_resource_extra.sh
  bin/resource_arsenal.hpp
  addons/virtual_arsenal.pbo
  addons/virtual_arsenal_src/
  Missions/VirtualArsenal.Intro/
mods/tools/pack_pbo.py
mods/tools/scan_weapons.py
```

## Notes

- Runtime SQF cannot enumerate `configFile >> CfgWeapons` like Arma 3; the catalog is
  generated offline. Re-run `pack.sh` after adding weapon mods.
- OFP often uses the same classname for a weapon and its magazine (`addMagazine "AK74"`).
- Game assets used for the catalog live outside git (`~/cwr-game/Remaster`).
