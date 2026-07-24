# AGENTS.md

## Cursor Cloud specific instructions

This repo is the **Poseidon** engine for *Arma: Cold War Assault - Remastered*: C++20/CMake
(`apps/`, `engine/`), Rust crates (`mserver/`, `engine/Trident`), and a Python Blender addon
(`apps/tools/BlenderAddon`). Primary build docs: `README.md`, `build.sh`, `mserver/README.md`.

### Environment (Ubuntu 22.04)

Cloud agents use the Dockerfile in `.cursor/` (`ubuntu:22.04`). After checkout, `.cursor/install.sh`
ensures rustup stable and a vcpkg checkout at `$VCPKG_ROOT` (default `$HOME/vcpkg`).

- `VCPKG_ROOT` must be set (also exported via `~/.cursor-env.sh`) — CMake presets fail without it.
- System packages include Clang, Ninja, ccache, autotools, OpenGL/X11/Wayland headers, and `libssl-dev`.
- CMake is Kitware **≥ 3.25** (Ubuntu jammy's apt cmake is too old for this tree).

### C++ engine

```sh
export VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"
cmake --preset linux-x64-clang-rwdi
cmake --build build/linux-x64-clang-rwdi
# or: ./build.sh
```

- First configure runs `vcpkg install` for the ports in `vcpkg.json` (slow); later runs are cached.
- Headless tests: `ALSOFT_DRIVERS=null ctest --test-dir build/linux-x64-clang-rwdi --output-on-failure`
  (without `ALSOFT_DRIVERS=null`, OpenAL tests fail — no audio device).
- Game clients need proprietary data under `packages/Demo` (Steam/APL-SA, not in this repo).

### Rust (PAPA BEAR / Trident)

Each crate is a **standalone** Cargo project (no workspace root). Use `--manifest-path`:

```sh
cargo run --manifest-path mserver/MasterService/Cargo.toml -- server --dev
# http://127.0.0.1:8080  (--dev seeds demo data)
cargo fmt --check --manifest-path mserver/MasterService/Cargo.toml
cargo clippy --manifest-path mserver/MasterService/Cargo.toml
```

Same pattern for `mserver/Client`, `mserver/CLI`, `mserver/Archive`, and `engine/Trident`.

### Blender addon

```sh
uv sync --project apps/tools/BlenderAddon   # if uv is available
```

Addon tests expect a built `PoseidonFormats` shared library; the Makefile's
`../../tests/BlenderAddon` path is not present in this locked upstream tree.
