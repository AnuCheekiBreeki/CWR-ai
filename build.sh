#!/usr/bin/env bash
# Single entry-point build script for Poseidon / CWR.
#
# Usage:
#   ./build.sh                  # configure (if needed) + build default targets
#   ./build.sh PoseidonServer   # build one or more CMake targets
#   ./build.sh --configure-only
#   ./build.sh --help
#
# Environment:
#   PRESET   — CMake preset (auto-detected by OS if unset)
#   JOBS     — parallel build jobs (default: nproc)
#   VCPKG_ROOT — required; path to vcpkg checkout
#
# Examples:
#   PRESET=linux-x64-clang-rwdi ./build.sh
#   ./build.sh PoseidonGame PoseidonServer PoseidonTests

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

default_preset() {
    case "$(uname -s)" in
        Linux*) echo "linux-x64-clang-rwdi" ;;
        MINGW*|MSYS*|CYGWIN*) echo "win-x64-clang-rwdi" ;;
        Darwin*)
            echo "error: macOS is not a supported build target; use Linux or Windows preset explicitly via PRESET=" >&2
            exit 1
            ;;
        *)
            echo "error: unsupported OS: $(uname -s)" >&2
            exit 1
            ;;
    esac
}

usage() {
    cat <<'EOF'
Usage: ./build.sh [options] [TARGET ...]

Configure and build the project using CMake presets from CMakePresets.json.

Options:
  --configure-only   Run cmake --preset only, do not build
  --trident          Also build Trident test runner (cargo)
  --clean            Remove build directory for the active preset before configure
  -h, --help         Show this help

Environment:
  PRESET      CMake preset name (default: OS-specific RelWithDebInfo preset)
  JOBS        Parallel build jobs (default: nproc or 4)
  VCPKG_ROOT  Path to vcpkg (required)

Default targets (when none specified):
  PoseidonGame, PoseidonServer, PoseidonTools, PoseidonTests
EOF
}

require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "error: required command not found: $1" >&2
        exit 1
    fi
}

PRESET="${PRESET:-$(default_preset)}"
BUILD_DIR="$ROOT/build/$PRESET"
JOBS="${JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
CONFIGURE_ONLY=0
BUILD_TRIDENT=0
CLEAN=0
TARGETS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --configure-only)
            CONFIGURE_ONLY=1
            shift
            ;;
        --trident)
            BUILD_TRIDENT=1
            shift
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            TARGETS+=("$@")
            break
            ;;
        -*)
            echo "error: unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
        *)
            TARGETS+=("$1")
            shift
            ;;
    esac
done

if [[ ${#TARGETS[@]} -eq 0 ]]; then
    TARGETS=(PoseidonGame PoseidonServer PoseidonTools PoseidonTests)
fi

require_cmd cmake
require_cmd ninja

if [[ -z "${VCPKG_ROOT:-}" ]]; then
    echo "error: VCPKG_ROOT is not set. Point it to a vcpkg checkout with the repo baseline." >&2
    echo "  export VCPKG_ROOT=/path/to/vcpkg" >&2
    exit 1
fi

if [[ ! -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]]; then
    echo "error: VCPKG_ROOT does not look like vcpkg: $VCPKG_ROOT" >&2
    exit 1
fi

echo "==> preset:  $PRESET"
echo "==> build:   $BUILD_DIR"
echo "==> jobs:    $JOBS"
echo "==> targets: ${TARGETS[*]}"

if [[ "$CLEAN" -eq 1 ]]; then
    echo "==> cleaning $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

echo "==> configuring"
cmake --preset "$PRESET"

if [[ "$CONFIGURE_ONLY" -eq 1 ]]; then
    echo "==> configure done"
    exit 0
fi

echo "==> building"
cmake --build "$BUILD_DIR" --parallel "$JOBS" --target "${TARGETS[@]}"

if [[ "$BUILD_TRIDENT" -eq 1 ]]; then
    require_cmd cargo
    echo "==> building Trident (tri)"
    cargo build --manifest-path "$ROOT/engine/Trident/Cargo.toml"
fi

echo "==> done"
