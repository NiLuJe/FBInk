#!/bin/bash -e

# Script to generate debian packages for FBInk.
PACKAGE="FBInk"
AUTHOR="NiLuJe <niluje@gmail.com>"
DESC_1="FrameBuffer eInker"
DESC_2="a small tool to print text and images to an eInk Linux framebuffer."

# Call this file from Makefile -------
# Use $(OUT_DIR) as the first argument
# and target arch as the second argument.
ROOTFS_BASE="${1}"
TARGET_ARCH="${2}"

# Temp stuff.
PKG_PATH="${ROOTFS_BASE}/pkg"
BIN_PATH="${PKG_PATH}/usr/bin"
DEB_PATH="${PKG_PATH}/DEBIAN"
CONTROL="${DEB_PATH}/control"

# Get version from git if posible. Fallback to defined FBINK_VERSION
# Note: version string must start with a number
FALLBACK_VERSION="$(grep 'define FBINK_VERSION' fbink_internal.h | cut -f2 -d\" | cut -f2 -dv)"
VERSION="$(git describe 2>/dev/null | sed 's/[^0-9]*//' || echo ${FALLBACK_VERSION})"

# package name
FULL_NAME="${PACKAGE}-${VERSION}-${TARGET_ARCH}.deb"

command_exists () {
  type "$1" >/dev/null 2>/dev/null
}

# Run only if dpkg-deb exists
COMMAND="dpkg-deb"
if command_exists "$COMMAND"; then
    echo "Building ${FULL_NAME}"
    mkdir -p ${BIN_PATH} ${DEB_PATH}
    echo "Package: ${PACKAGE}" > "$CONTROL"
    echo "Version: ${VERSION}" >> "$CONTROL"
    echo "Section: base" >> "$CONTROL"
    echo "Priority: optional" >> "$CONTROL"
    echo "Architecture: ${TARGET_ARCH}" >> "$CONTROL"
    echo "Maintainer: ${AUTHOR}" >> "$CONTROL"
    echo "Description: ${DESC_1}" >> "$CONTROL"
    echo " ${DESC_2}" >> "$CONTROL"
    cp -p ${ROOTFS_BASE}/fbink ${BIN_PATH}
    dpkg-deb -b ${PKG_PATH} ${ROOTFS_BASE}/${FULL_NAME}
    rm -rf ${PKG_PATH}
else
    echo "${COMMAND} not found, skipping target ${FULL_NAME}"
fi

exit 0
