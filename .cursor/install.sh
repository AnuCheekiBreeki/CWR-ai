#!/usr/bin/env bash
# Idempotent Cursor Cloud update script for CWR / Poseidon on Ubuntu 22.04.
# Runs from the repository root after each agent boot.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

export PATH="/usr/local/bin:${HOME}/.cargo/bin:${PATH}"
export VCPKG_ROOT="${VCPKG_ROOT:-${HOME}/vcpkg}"
export CCACHE_DIR="${CCACHE_DIR:-${HOME}/.ccache}"

mkdir -p "${CCACHE_DIR}"

echo "==> OS: $(. /etc/os-release && echo "$PRETTY_NAME")"
echo "==> cmake: $(cmake --version | head -n1)"
echo "==> clang: $(clang --version | head -n1)"
echo "==> ninja: $(ninja --version)"

# --- Rust (rustup stable) -----------------------------------------------------
if ! command -v rustup >/dev/null 2>&1; then
    echo "==> installing rustup"
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs \
        | sh -s -- -y --default-toolchain stable --profile minimal
fi

# shellcheck source=/dev/null
source "${HOME}/.cargo/env" 2>/dev/null || true
rustup default stable
rustup component add rustfmt clippy >/dev/null
echo "==> rustc: $(rustc --version)"
echo "==> cargo: $(cargo --version)"

# --- vcpkg (matches vcpkg.json builtin-baseline when available) ---------------
VCPKG_BASELINE="$(sed -n 's/.*"builtin-baseline"[[:space:]]*:[[:space:]]*"\([0-9a-fA-F]*\)".*/\1/p' "${ROOT}/vcpkg.json" | head -n 1 || true)"

ensure_vcpkg() {
    local need_bootstrap=0

    if [[ ! -d "${VCPKG_ROOT}/.git" ]]; then
        echo "==> cloning vcpkg into ${VCPKG_ROOT}"
        rm -rf "${VCPKG_ROOT}"
        mkdir -p "${VCPKG_ROOT}"
        git -C "${VCPKG_ROOT}" init
        git -C "${VCPKG_ROOT}" remote add origin https://github.com/microsoft/vcpkg.git
        need_bootstrap=1
    fi

    if [[ -n "${VCPKG_BASELINE}" ]]; then
        if ! git -C "${VCPKG_ROOT}" rev-parse --verify "${VCPKG_BASELINE}^{commit}" >/dev/null 2>&1; then
            echo "==> fetching vcpkg baseline ${VCPKG_BASELINE}"
            git -C "${VCPKG_ROOT}" fetch --depth 1 origin "${VCPKG_BASELINE}"
        fi
        local head
        head="$(git -C "${VCPKG_ROOT}" rev-parse HEAD 2>/dev/null || true)"
        if [[ "${head}" != "${VCPKG_BASELINE}"* ]]; then
            git -C "${VCPKG_ROOT}" checkout --force "${VCPKG_BASELINE}"
            need_bootstrap=1
        fi
    elif [[ ! -x "${VCPKG_ROOT}/vcpkg" ]]; then
        echo "==> fetching vcpkg master (no baseline in vcpkg.json)"
        git -C "${VCPKG_ROOT}" fetch --depth 1 origin master
        git -C "${VCPKG_ROOT}" checkout --force FETCH_HEAD
        need_bootstrap=1
    fi

    if [[ ! -x "${VCPKG_ROOT}/vcpkg" || "${need_bootstrap}" -eq 1 ]]; then
        echo "==> bootstrapping vcpkg"
        "${VCPKG_ROOT}/bootstrap-vcpkg.sh" -disableMetrics
    fi
}

ensure_vcpkg
echo "==> VCPKG_ROOT=${VCPKG_ROOT}"

# Keep shell profile in sync for agent terminals.
PROFILE_SNIPPET='. "$HOME/.cursor-env.sh" 2>/dev/null || true'
if [[ -f "${HOME}/.bashrc" ]] && ! grep -Fq '.cursor-env.sh' "${HOME}/.bashrc"; then
    echo "${PROFILE_SNIPPET}" >> "${HOME}/.bashrc"
fi
if [[ ! -f "${HOME}/.cursor-env.sh" ]]; then
    printf '%s\n' \
        'export PATH="/usr/local/bin:$HOME/.cargo/bin:$PATH"' \
        'export VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"' \
        'export CCACHE_DIR="${CCACHE_DIR:-$HOME/.ccache}"' \
        > "${HOME}/.cursor-env.sh"
fi

# Ensure VCPKG_ROOT is exported for the current install session and child agents.
grep -q 'VCPKG_ROOT' "${HOME}/.cursor-env.sh" || true
{
    echo "export PATH=\"/usr/local/bin:\$HOME/.cargo/bin:\$PATH\""
    echo "export VCPKG_ROOT=\"${VCPKG_ROOT}\""
    echo "export CCACHE_DIR=\"${CCACHE_DIR}\""
} > "${HOME}/.cursor-env.sh"

echo "==> install complete"
