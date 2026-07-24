# AGENTS.md

## Cursor Cloud specific instructions

This repo is the **Poseidon** engine for *Arma: Cold War Assault - Remastered*: a C++20/CMake
game engine (`apps/`, `engine/`) plus Rust crates (`mserver/`, `engine/Trident`) and a Python
Blender addon (`apps/tools/BlenderAddon`). Build/run commands live in `README.md`, `mserver/README.md`,
and `apps/tools/BlenderAddon/Makefile`; only the non-obvious caveats are captured here.

### Toolchain (already provisioned in the VM snapshot)
- `VCPKG_ROOT=$HOME/vcpkg` is exported in `~/.bashrc` — the CMake presets fail to configure without it.
- Rust defaults to **rustup `stable`** (installed via `rustup default stable`). The image's original
  Cargo (1.83) is too old: a transitive dependency in the committed `Cargo.lock` files requires
  `edition2024` (Cargo ≥ 1.85). Do not pin back to 1.83.
- System build deps are installed at the OS level: `ninja`, `ccache`, `clang`/`clang++`/`clang-format` 18,
  `g++`/`libstdc++-dev`, autotools (`autoconf automake libtool autoconf-archive`), `libssl-dev`,
  and OpenGL dev libs (`libgl1-mesa-dev libglx-dev libopengl-dev`). `uv` is at `~/.local/bin`.

### C++ engine (core product)
- Configure + build: `cmake --preset linux-x64-clang-rwdi && cmake --build build/linux-x64-clang-rwdi`.
- The **first** configure runs `vcpkg install` for ~28 ports (sdl3, openal-soft, glslang, curl, imgui, …)
  and is slow; results are cached under `$HOME/vcpkg` + `build/.../vcpkg_installed` afterwards.
- Tests: `cd build/linux-x64-clang-rwdi && ALSOFT_DRIVERS=null ctest`. **`ALSOFT_DRIVERS=null` is required
  headless** — without it, 5 `OAL - …` OpenAL tests fail with "OpenAL not available" because the VM has no
  audio device. With it, all tests pass.
- Built binaries land in `build/linux-x64-clang-rwdi/apps/...`. `PoseidonTools` and `PoseidonEvaluator`
  run standalone (e.g. `PoseidonEvaluator --eval '2 + 3 * 4'`).
- The game clients (`PoseidonGame`, `PoseidonGameDemo`, `PoseidonServer`) **require proprietary game data**
  under `packages/Demo` (APL-SA, Steam-only, git-ignored, not in this repo). They cannot run without it,
  so the runnable end-to-end target here is the PAPA BEAR master service below.

### PAPA BEAR master service (Rust, runnable)
- Run: `cargo run --manifest-path mserver/MasterService/Cargo.toml -- server --dev` →
  serves a web UI + JSON API on `http://127.0.0.1:8080` and seeds demo data (`--dev`).
  Useful routes: `/` (landing), `/browser`, `/mods`, `/healthz`, `/v1/servers`,
  `POST /v1/servers/register`, `/v1/meta/summary`, `/openapi/v1.yaml`. It writes a SQLite DB under
  `mserver/MasterService/tmp/`.
- Each crate in `mserver/` (`MasterService`, `Client`, `CLI`, `Archive`) and `engine/Trident` is a
  **standalone Cargo project — there is no workspace root**. Build/test/lint each with
  `--manifest-path <crate>/Cargo.toml`. The `CLI` crate links `openssl-sys` (needs `libssl-dev`).
- Lint (as CI does): `cargo fmt --check` and `cargo clippy` per crate. The manifests set
  `clippy::all = deny` (passes); the newer stable clippy also emits extra warn-level pedantic/nursery
  lints — those are expected and non-blocking.

### Blender addon (Python)
- `uv sync --project apps/tools/BlenderAddon` installs dev deps (pytest, ruff).
- Lint the addon source directly: `uv run --project apps/tools/BlenderAddon ruff check io_import_p3d`
  (run from `apps/tools/BlenderAddon`). Note: the `Makefile`'s `check`/`test` targets reference
  `../../tests/BlenderAddon`, but that test directory is **not present in this repo**, so those targets
  error out; the addon's pytest suite also depends on a built `PoseidonFormats.so`.
