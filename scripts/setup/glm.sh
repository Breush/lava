#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

VERSION="$1"

# -----

cd "${ROOT_DIR}/.tmp"
unzip -o glm_${VERSION}.zip
cp -r glm-${VERSION}/glm ../include
