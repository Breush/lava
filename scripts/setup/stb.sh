#!/bin/bash

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

#===== STB

cd "${ROOT_DIR}/.tmp"
mkdir -p ../include/stb
cp -R stb/stb_image.h ../include/stb/stb_image.h
