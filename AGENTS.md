# AGENTS.md

## Cursor Cloud specific instructions

### VNC desktop environment

Cursor Cloud Agents with computer use provision a full desktop automatically at startup. No manual VNC setup is required.

| Component | Details |
|-----------|---------|
| Display | `:1` (`DISPLAY=:1` is set in the shell) |
| VNC server | TigerVNC on port **5901** (localhost only) |
| Web access | noVNC on port **26058** — open `http://cursor:26058/vnc.html` from the agent UI |
| Desktop | XFCE 4.18 with Plank dock |
| Resolution | 1920×1200 @ 96 DPI |

To launch a GUI application from the terminal:

```sh
DISPLAY=:1 <command>
```

The desktop uses software rendering (`LIBGL_ALWAYS_SOFTWARE=1`) since there is no hardware GPU. This is sufficient for most GUI tools and basic OpenGL/WebGL testing.

Desktop initialization logs are written to `/tmp/container-init.log`. VNC session logs are in `~/.vnc/`.

### C++ engine build notes

- `VCPKG_ROOT` must point to a full (non-shallow) vcpkg clone. Presets read `$env{VCPKG_ROOT}`.
- Configure + build (Linux): `cmake --preset linux-x64-clang-rwdi` then `cmake --build build/linux-x64-clang-rwdi`.
- Do **not** install `libibus-1.0-dev` — it breaks the vcpkg SDL3 static link.
- `libstdc++-14-dev` is required because clang uses the gcc-14 toolchain.
- GUI game targets need external Steam Demo game data (not in this repo).
