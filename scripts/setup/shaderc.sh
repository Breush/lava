#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"
NAME=$(cat "$ROOT_DIR/shaderc.lua" | grep NAME -m 1 | cut -d '"' -f 2)
VERSION=$(cat "$ROOT_DIR/shaderc.lua" | grep VERSION -m 1 | cut -d '"' -f 2)

mkdir -p "${ROOT_DIR}/.tmp/shaderc"
cd "${ROOT_DIR}/.tmp/shaderc"

# ----- Download and unzip

if [ ! -f ${VERSION}.zip ]; then
    echo "... Downloading ${NAME}..."
    curl -L https://github.com/google/shaderc/archive/v${VERSION}.zip -o ${VERSION}.zip
fi

if [ ! -d ${VERSION} ]; then
    unzip -n ${VERSION}.zip -d ${VERSION}
fi

# ----- Compile

cd ${VERSION}/*
FOLDER=`pwd`

echo "... Syncing ${NAME} dependencies..."
python ./utils/git-sync-deps

mkdir -p build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${FOLDER}/build/install -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DSHADERC_SKIP_TESTS=ON -DDISABLE_EXCEPTIONS=ON
make -j 2

# ----- Install

make install
cp -R install/include/shaderc ${ROOT_DIR}/include
cp install/lib/libshaderc_combined.a ${ROOT_DIR}/lib
echo "OK" > ${ROOT_DIR}/.tmp/shaderc/${VERSION}.txt
