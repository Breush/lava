#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"
NAME=$(cat "$ROOT_DIR/openvr.lua" | grep NAME -m 1 | cut -d '"' -f 2)
VERSION=$(cat "$ROOT_DIR/openvr.lua" | grep VERSION -m 1 | cut -d '"' -f 2)

mkdir -p "${ROOT_DIR}/.tmp/openvr"
cd "${ROOT_DIR}/.tmp/openvr"

# ----- Download and unzip

if [ ! -f ${VERSION}.zip ]; then
    echo "... Downloading ${NAME}..."
    curl -L https://github.com/ValveSoftware/openvr/archive/v${VERSION}.zip -o ${VERSION}.zip
fi

if [ ! -d ${VERSION} ]; then
    unzip -n ${VERSION}.zip -d ${VERSION}
fi

# ----- Install

cd ${VERSION}/*

mkdir -p ${ROOT_DIR}/include/openvr
cp -R headers/* ${ROOT_DIR}/include/openvr

if [ `uname -o` == "Msys" ]; then
    cp -R lib/win64/* ${ROOT_DIR}/lib
    cp -R bin/win64/* ${ROOT_DIR}/lib
else
    cp -R lib/linux64/* ${ROOT_DIR}/lib
fi

echo "OK" > ${ROOT_DIR}/.tmp/openvr/${VERSION}.txt
