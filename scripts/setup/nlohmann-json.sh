#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"
NAME=$(cat "$ROOT_DIR/nlohmann-json.lua" | grep NAME -m 1 | cut -d '"' -f 2)
VERSION=$(cat "$ROOT_DIR/nlohmann-json.lua" | grep VERSION -m 1 | cut -d '"' -f 2)

mkdir -p "${ROOT_DIR}/.tmp/nlohmann-json"
cd "${ROOT_DIR}/.tmp/nlohmann-json"

# ----- Download

if [ ! -f ${VERSION}.hpp ]; then
    echo "... Downloading ${NAME}..."
    curl -L https://github.com/nlohmann/json/releases/download/v${VERSION}/json.hpp -o ${VERSION}.hpp
fi

# ----- Install

mkdir -p ${ROOT_DIR}/include/nlohmann
cp ${VERSION}.hpp ${ROOT_DIR}/include/nlohmann/json.hpp
echo "OK" > ${ROOT_DIR}/.tmp/nlohmann-json/${VERSION}.txt
