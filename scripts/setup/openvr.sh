#!/bin/bash

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

VERSION="$1"

MAKE="make"
if [ `uname -o` == "Msys" ]; then
    MAKE="mingw32-make"
fi

# Setting up
cd "${ROOT_DIR}/.tmp"
unzip -n openvr_${VERSION}.zip
cd openvr-${VERSION}
FOLDER=`pwd`

# Copying libs
cd "${FOLDER}"
mkdir -p ../../include/openvr
cp -R headers/* ../../include/openvr
cp -R lib/linux64/* ../../lib
