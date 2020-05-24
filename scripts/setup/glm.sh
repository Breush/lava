#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"
NAME=$(cat "$ROOT_DIR/glm.lua" | grep NAME -m 1 | cut -d '"' -f 2)
VERSION=$(cat "$ROOT_DIR/glm.lua" | grep VERSION -m 1 | cut -d '"' -f 2)

mkdir -p "${ROOT_DIR}/.tmp/glm"
cd "${ROOT_DIR}/.tmp/glm"

# ----- Download and unzip

if [ ! -f ${VERSION}.zip ]; then
    echo "... Downloading ${NAME}..."
    curl -L https://github.com/g-truc/glm/archive/${VERSION}.zip -o ${VERSION}.zip
fi

if [ ! -d ${VERSION} ]; then
    unzip -n ${VERSION}.zip -d ${VERSION}
fi

# ----- Install

cd ${VERSION}/*

cp -R glm ${ROOT_DIR}/include
echo "OK" > ${ROOT_DIR}/.tmp/glm/${VERSION}.txt
