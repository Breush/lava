#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"
NAME=$(cat "$ROOT_DIR/bullet.lua" | grep NAME -m 1 | cut -d '"' -f 2)
VERSION=$(cat "$ROOT_DIR/bullet.lua" | grep VERSION -m 1 | cut -d '"' -f 2)

mkdir -p "${ROOT_DIR}/.tmp/bullet"
cd "${ROOT_DIR}/.tmp/bullet"

# ----- Download and unzip

if [ ! -f ${VERSION}.zip ]; then
    echo "... Downloading ${NAME}..."
    curl -L https://github.com/bulletphysics/bullet3/archive/${VERSION}.zip -o ${VERSION}.zip
fi

if [ ! -d ${VERSION} ]; then
    unzip -n ${VERSION}.zip -d ${VERSION}
fi

# ----- Compile

cd ${VERSION}/*
FOLDER=`pwd`

mkdir -p build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${FOLDER}/build/install -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_BULLET2_DEMOS=OFF -DBUILD_CLSOCKET=OFF -DBUILD_CPU_DEMOS=OFF -DBUILD_ENET=OFF -DBUILD_OPENGL3_DEMOS=OFF -DBUILD_PYBULLET=OFF -DBUILD_UNIT_TESTS=OFF
make -j 2

# ----- Install

make install
cp -R install/include/bullet ${ROOT_DIR}/include
cp install/lib/libBulletDynamics.a install/lib/libBulletCollision.a install/lib/libLinearMath.a ${ROOT_DIR}/lib
echo "OK" > ${ROOT_DIR}/.tmp/bullet/${VERSION}.txt
