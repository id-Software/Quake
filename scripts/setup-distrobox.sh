#!/usr/bin/env bash
# Set up a Fedora development container for Foreshock on Bazzite.
#
# Bazzite's base system is read-only, so compilers and libraries live in a
# Distrobox container instead of on the host. Your home directory (and this
# repo) is shared with the container, so you can edit files on the host and
# build inside the container.
#
# Usage:
#   ./scripts/setup-distrobox.sh          # create (or update) the container
#   distrobox enter foreshock-dev         # open a shell inside it
#
# Safe to run more than once: an existing container is reused and the
# package install is simply repeated.

set -euo pipefail

CONTAINER_NAME="${FORESHOCK_CONTAINER:-foreshock-dev}"
IMAGE="${FORESHOCK_IMAGE:-registry.fedoraproject.org/fedora-toolbox:latest}"

PACKAGES=(
    # Compilers and build tools
    gcc
    gcc-c++
    clang
    clang-tools-extra   # clang-format, clang-tidy
    make
    cmake
    ninja-build
    meson
    pkgconf-pkg-config
    git

    # Libraries the modernised engine will use
    SDL2-devel          # video, input, audio, controllers
    mesa-libGL-devel    # OpenGL renderer

    # Debugging
    gdb
    valgrind
)

if ! command -v distrobox >/dev/null 2>&1; then
    echo "error: distrobox not found. It ships with Bazzite; on other systems install it first." >&2
    exit 1
fi

if distrobox list --no-color 2>/dev/null | awk -F'|' '{gsub(/ /, "", $2); print $2}' | grep -qx "$CONTAINER_NAME"; then
    echo "Container '$CONTAINER_NAME' already exists; reusing it."
else
    echo "Creating container '$CONTAINER_NAME' from $IMAGE ..."
    distrobox create --yes --name "$CONTAINER_NAME" --image "$IMAGE"
fi

echo "Installing development packages inside '$CONTAINER_NAME' ..."
distrobox enter "$CONTAINER_NAME" -- sudo dnf install -y "${PACKAGES[@]}"

cat <<EOF

Done. Enter the container with:

    distrobox enter $CONTAINER_NAME

For Flatpak packaging, install flatpak-builder on the host instead:

    flatpak install -y flathub org.flatpak.Builder
EOF
