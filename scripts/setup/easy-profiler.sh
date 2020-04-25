#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

VERSION="$1"

# Setting up
cd "${ROOT_DIR}/.tmp"
unzip -n easy_profiler_${VERSION}.zip
cd easy_profiler-${VERSION}
FOLDER=`pwd`

# Builder
cd "${FOLDER}"
mkdir -p build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make -j 2

# Copying libs
cd "${FOLDER}"
cp -R easy_profiler_core/include/* ../../include/
cp -R build/bin/libeasy_profiler.so ../../lib
cp -R build/bin/profiler_gui ../../bin
