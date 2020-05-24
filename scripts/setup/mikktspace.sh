#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"
REPOSITORY=$(cat "$ROOT_DIR/mikktspace.lua" | grep REPOSITORY -m 1 | cut -d '"' -f 2)

mkdir -p "${ROOT_DIR}/.tmp/mikktspace"
cd "${ROOT_DIR}/.tmp/mikktspace"

# ----- Download

if [ ! -d repository ]; then
    git clone --depth=1 ${REPOSITORY} repository
fi

# ----- Install

cd repository

mkdir -p ${ROOT_DIR}/include/mikktspace
cp mikktspace.* ${ROOT_DIR}/include/mikktspace/
echo "OK" > ${ROOT_DIR}/.tmp/mikktspace/repository.txt
