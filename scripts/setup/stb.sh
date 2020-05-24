#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"
REPOSITORY=$(cat "$ROOT_DIR/stb.lua" | grep REPOSITORY -m 1 | cut -d '"' -f 2)

mkdir -p "${ROOT_DIR}/.tmp/stb"
cd "${ROOT_DIR}/.tmp/stb"

# ----- Download

if [ ! -d repository ]; then
    git clone --depth=1 ${REPOSITORY} repository
fi

# ----- Install

cd repository

mkdir -p ${ROOT_DIR}/include/stb
cp stb_image.h stb_image_write.h stb_truetype.h stb_vorbis.c ${ROOT_DIR}/include/stb
echo "OK" > ${ROOT_DIR}/.tmp/stb/repository.txt
