#!/bin/bash

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

VERSION="$1"

MAKE="make"
if [ `uname -o` == "Msys" ]; then
    MAKE="mingw32-make"
fi

# Setting up
cd "${ROOT_DIR}/.tmp"
unzip -n easy_profiler_${VERSION}.zip
cd easy_profiler-${VERSION}
FOLDER=`pwd`

# Builder
cd "${FOLDER}"
mkdir -p build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_MAKE_PROGRAM="${MAKE}" -DCMAKE_POSITION_INDEPENDENT_CODE=ON
${MAKE} -j 2

# Copying libs
cd "${FOLDER}"
cp -R easy_profiler_core/include/* ../../include/
cp -R build/bin/libeasy_profiler.so ../../lib
cp -R build/bin/profiler_gui ../../bin
