# Cursor Cloud agent environment for CWR / Poseidon.
# Base: Ubuntu 22.04 (jammy) — matches the project's Linux CI target.
FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
ARG CMAKE_VERSION=3.31.6

# Avoid interactive tzdata prompts during package install.
ENV TZ=UTC \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        wget \
        gnupg \
        git \
        sudo \
        openssh-client \
        build-essential \
        ninja-build \
        ccache \
        pkg-config \
        python3 \
        python3-pip \
        python3-venv \
        zip \
        unzip \
        tar \
        xz-utils \
        # Compilers (project uses Clang via linux-x64-clang presets)
        clang \
        clang-format \
        clang-tidy \
        lld \
        llvm \
        g++ \
        # Autotools (needed by some vcpkg ports)
        autoconf \
        autoconf-archive \
        automake \
        libtool \
        libltdl-dev \
        # SSL / crypto (curl vcpkg feature + Rust openssl-sys)
        libssl-dev \
        # OpenGL + X11 / Wayland (SDL3 and related vcpkg ports)
        libgl1-mesa-dev \
        libglx-dev \
        libopengl-dev \
        libx11-dev \
        libxext-dev \
        libxrandr-dev \
        libxcursor-dev \
        libxi-dev \
        libxtst-dev \
        libxinerama-dev \
        libxkbcommon-dev \
        libwayland-dev \
        wayland-protocols \
        # Misc useful agent tooling
        jq \
        ripgrep \
        file \
        less \
        nano \
    && rm -rf /var/lib/apt/lists/*

# Ubuntu 22.04 ships CMake 3.22; this project requires >= 3.25.
RUN set -eux; \
    arch="$(dpkg --print-architecture)"; \
    case "$arch" in \
      amd64) cmake_arch=x86_64 ;; \
      arm64) cmake_arch=aarch64 ;; \
      *) echo "unsupported arch: $arch" >&2; exit 1 ;; \
    esac; \
    curl -fsSL "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-${cmake_arch}.tar.gz" \
      | tar -xz -C /opt; \
    ln -sfn "/opt/cmake-${CMAKE_VERSION}-linux-${cmake_arch}" /opt/cmake; \
    ln -sf /opt/cmake/bin/cmake /usr/local/bin/cmake; \
    ln -sf /opt/cmake/bin/cpack /usr/local/bin/cpack; \
    ln -sf /opt/cmake/bin/ctest /usr/local/bin/ctest; \
    cmake --version

# Non-root `ubuntu` user with passwordless sudo (required by Cursor Cloud Agents).
RUN id -u ubuntu >/dev/null 2>&1 || useradd -m -s /bin/bash ubuntu \
    && usermod -aG sudo ubuntu \
    && echo "ubuntu ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/ubuntu \
    && chmod 0440 /etc/sudoers.d/ubuntu

USER ubuntu
WORKDIR /home/ubuntu

# Persist toolchain paths for interactive shells.
RUN printf '%s\n' \
      'export PATH="/usr/local/bin:$HOME/.cargo/bin:$PATH"' \
      'export VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"' \
      'export CCACHE_DIR="${CCACHE_DIR:-$HOME/.ccache}"' \
      > /home/ubuntu/.cursor-env.sh \
    && echo '. "$HOME/.cursor-env.sh"' >> /home/ubuntu/.bashrc

ENV PATH="/usr/local/bin:/home/ubuntu/.cargo/bin:${PATH}" \
    VCPKG_ROOT=/home/ubuntu/vcpkg \
    CCACHE_DIR=/home/ubuntu/.ccache

# Do not COPY the project — Cursor checks out the correct commit at runtime.
CMD ["/bin/bash"]
