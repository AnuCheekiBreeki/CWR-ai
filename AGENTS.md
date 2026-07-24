# AGENTS.md

## Cursor Cloud specific instructions

This repository is the **Poseidon** engine / *Arma: Cold War Assault - Remastered* source
release. It contains three independently buildable parts. Standard build/test/run commands
already live in `README.md`, `tests/README.md`, `mserver/README.md`, and
`apps/tools/BlenderAddon/README.md` / `Makefile` — prefer those. The notes below only capture
non-obvious, durable caveats for this environment.

### Components
- **C++ engine + apps** (CMake + Ninja + Clang + vcpkg): the core product (game clients,
  dedicated server, tools, unit tests). Presets in `CMakePresets.json`.
- **Rust master server tools** (`mserver/*`, cargo): the PAPA BEAR HTTP directory service
  (`papa-bear-master-service`) plus CLI/client crates. Also `engine/Trident` (`tri`) test runner.
- **Python Blender addon** (`apps/tools/BlenderAddon`, uv + ruff + pytest).

### Rust (`mserver/*`, `engine/Trident`)
- Requires a Rust toolchain that supports **edition 2024** (Rust ≥ 1.85). The stock 1.83 is too
  old: the pinned `Cargo.lock` (e.g. `clap_builder 4.6.0`) fails with
  "feature `edition2024` is required". `rustup default stable` is set here to satisfy this.
- Build/run the master service (SQLite is embedded, no DB server needed):
  `cargo run --manifest-path mserver/MasterService/Cargo.toml --bin papa-bear-master-service -- server --listen 127.0.0.1:8080 --db tmp/papa-bear.sqlite3 --dev`
  (`--dev` seeds demo servers/mods). Tests: `cargo test --manifest-path mserver/MasterService/Cargo.toml`.
- Web UI: `http://127.0.0.1:8080/browser`. The "verify" filter defaults to **public**, which
  hides `self_reported` (un-probed) servers, so a freshly `--dev`-seeded or newly registered
  server shows an empty table until you switch the filter to **all**. API is under `/v1/...`,
  OpenAPI at `/openapi/v1.yaml`.

### C++ engine (vcpkg)
- `VCPKG_ROOT=/opt/vcpkg` is exported in `~/.bashrc`. The presets read `$env{VCPKG_ROOT}`.
- vcpkg **must be a full (non-shallow) clone**: the `vcpkg.json` version `overrides` reference
  historical git trees that a `--depth 1` clone cannot unpack.
- Configure + build (Linux): `cmake --preset linux-x64-clang-rwdi` then
  `cmake --build build/linux-x64-clang-rwdi [--target <T>]`. The **first** configure compiles
  all vcpkg dependencies (~15-20 min on this VM); they are cached in `/opt/vcpkg` and the build
  dir afterward.
- **SDL3 link gotcha:** do NOT install `libibus-1.0-dev`. The vcpkg SDL3 port builds with
  `SDL_DBUS=OFF` but `SDL_IBUS=ON`; if IBus dev headers are present, `libSDL3.a` ends up with
  undefined `SDL_IBus_*` symbols and every executable link fails. Keep ibus headers absent.
- clang selects the gcc-14 toolchain, so `libstdc++-14-dev` must be installed (not just
  gcc-13's headers), otherwise `<vector>` / `-lstdc++` are not found.
- Unit tests run without game data: `ctest --test-dir build/linux-x64-clang-rwdi`. The SQF
  evaluator is a good no-data smoke test:
  `build/linux-x64-clang-rwdi/apps/tools/Evaluator/PoseidonEvaluator --eval '2 + 3 * 4'`.
- The GUI game targets (`PoseidonGame`, `PoseidonGameDemo`) build here but **cannot actually
  run** without external Steam Demo game data (not in this repo; see `README.md`). Trident
  integration tests are likewise disabled unless game data + `.trident.env` are provided.

### Python Blender addon
- Use `uv` (installed at `~/.local/bin/uv`). `uv sync` / `uv run ruff check io_import_p3d`
  from `apps/tools/BlenderAddon`. The pytest suite referenced by the `Makefile`
  (`../../tests/BlenderAddon`) is **not present** in this source release and additionally needs
  a built `PoseidonFormats.so` and game data, so `make test` has nothing to run here.
