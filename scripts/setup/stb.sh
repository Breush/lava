#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

#===== STB

cd "${ROOT_DIR}/.tmp"
mkdir -p ../include/stb
cp -R stb/stb_image.h ../include/stb/stb_image.h
cp -R stb/stb_image_write.h ../include/stb/stb_image_write.h
cp -R stb/stb_truetype.h ../include/stb/stb_truetype.h
cp -R stb/stb_vorbis.c ../include/stb/stb_vorbis.h
cp -R stb/stb_c_lexer.h ../include/stb/stb_c_lexer.h
