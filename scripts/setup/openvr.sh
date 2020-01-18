#!/bin/bash

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

VERSION=$(cat "$ROOT_DIR/openvr.lua" | grep VERSION -m 1 | cut -d '"' -f 2)

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
cp -R lib/win64/* ../../lib
