#!/bin/bash

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

VERSION="$1"

#===== Windows

if [ `uname -o` == "Msys" ]; then
    MAKE="mingw32-make"

    #===== Vulkan SDK

    cd "${ROOT_DIR}/.tmp"
    cp -R vulkan-sdk/Lib/* ../lib
    cp -R vulkan-sdk/Include/vulkan ../include

    mkdir -p ../etc/explicit_layer.d
    cp -R vulkan-sdk/Bin/*.{json,dll} ../etc/explicit_layer.d

    #===== shaderc

    FOLDER="./vulkan-sdk/shaderc"
    cd "${ROOT_DIR}/.tmp/${FOLDER}"
    mkdir -p build
    cd build
    CXXFLAGS=-fPIC cmake .. -G"Unix Makefiles" -DCMAKE_MAKE_PROGRAM="${MAKE}" -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    ${MAKE} -j 2 shaderc

    cd "${ROOT_DIR}/.tmp"
    cp -R ${FOLDER}/libshaderc/include/* ../include
    cp -R `find ${FOLDER}/build -type f \( -name *.a ! -wholename '*CMakeFiles/*' \)` ../lib

#===== Linux based

else
    MAKE="make"

    cd "${ROOT_DIR}/.tmp"
    tar -zxvf vulkan-sdk_${VERSION}.tar.gz

    #===== shaderc

    cd "${ROOT_DIR}/.tmp/${VERSION}"
    # Download all dependencies, and make install in x86_64/lib etc.
    # Use --debug to compile in debug mode
    ./vulkansdk shaderc

    #===== Vulkan SDK - copy files

    cd "${ROOT_DIR}/.tmp"
    FOLDER="${VERSION}/x86_64"
    cp -R ${FOLDER}/include/* ../include
    cp -R ${FOLDER}/etc/* ../etc
    cp -R ${FOLDER}/lib/* ../lib
fi
