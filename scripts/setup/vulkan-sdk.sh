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

    #===== Vulkan SDK

    cd "${ROOT_DIR}/.tmp"
    bash ./vulkan-sdk_${VERSION}.run
    FOLDER="VulkanSDK/${VERSION}/x86_64"
    cp -R ${FOLDER}/include/vulkan ../include
    cp -R ${FOLDER}/etc/* ../etc

    # @todo We're using libs from source folder for debugging,
    # but we should use the ones in x84_64/lib for release.
    mkdir -p ../source/vulkan
    FOLDER="VulkanSDK/${VERSION}/source"
    cp -R ${FOLDER}/lib/* ../lib
    cp -R ${FOLDER}/layers ../source/vulkan

    #===== shaderc

    FOLDER="VulkanSDK/${VERSION}/source/shaderc"
    cd "${ROOT_DIR}/.tmp/${FOLDER}"
    ./update_shaderc_sources.py
    cd src
    mkdir -p build
    cd build
    CXXFLAGS=-fPIC cmake .. -DCMAKE_MAKE_PROGRAM="${MAKE}" -G"Unix Makefiles"
    ${MAKE} -j 2 shaderc

    cd "${ROOT_DIR}/.tmp"
    cp -R ${FOLDER}/src/libshaderc/include/* ../include
    cp -R `find ${FOLDER} -type f \( -name *.a ! -wholename '*CMakeFiles/*' \)` ../lib
fi
