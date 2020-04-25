#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

#===== MikkTSpace

cd "${ROOT_DIR}/.tmp"
mkdir -p ../include/mikktspace
cp -R mikktspace/mikktspace.h ../include/mikktspace/mikktspace.h
cp -R mikktspace/mikktspace.c ../include/mikktspace/mikktspace.c
