#!/bin/bash

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

MAKE="make"
if [ `uname -o` == "Msys" ]; then
    MAKE="mingw32-make"
fi

# Setting up
cd "${ROOT_DIR}/.tmp"
unzip -n bullet_*.zip
cd bullet3-*
FOLDER=`pwd`

# Building
cd "${FOLDER}"
mkdir -p build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_MAKE_PROGRAM="${MAKE}" -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_BULLET2_DEMOS=OFF -DBUILD_CLSOCKET=OFF -DBUILD_CPU_DEMOS=OFF -DBUILD_ENET=OFF -DBUILD_OPENGL3_DEMOS=OFF -DBUILD_PYBULLET=OFF -DBUILD_UNIT_TESTS=OFF
${MAKE} -j 2

# Copying libs
cd "${FOLDER}"
cp -R src ../../include/bullet
cp -R `find build -name "*.a"` ../../lib
