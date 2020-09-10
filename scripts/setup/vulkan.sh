#!/bin/bash

set -e # exit if any command fails

cd "$(dirname "$0")"
ROOT_DIR="$(pwd)/../../external"

VERSION=$(cat "$ROOT_DIR/vulkan.lua" | grep VERSION -m 1 | cut -d '"' -f 2)

mkdir -p "${ROOT_DIR}/.tmp"
cd "${ROOT_DIR}/.tmp"
mkdir -p vulkan/${VERSION}

#===== Vulkan Headers

cd "${ROOT_DIR}/.tmp/vulkan/${VERSION}"

# ----- Download and unzip

HEADERS_VERSION=$(wget https://api.github.com/repos/KhronosGroup/Vulkan-Headers/git/refs/tags -q -O - | grep \"refs/tags/sdk-${VERSION} | tail -1 | cut -d'/' -f3 | cut -d'"' -f1)

if [ ! -f headers.zip ]; then
    echo "=> [vulkan/Headers] Downloading ${HEADERS_VERSION}"
    curl -L -f0 https://github.com/KhronosGroup/Vulkan-Headers/archive/${HEADERS_VERSION}.zip -o headers.zip
fi

if [ ! -d headers ]; then
    echo "=> [vulkan/Headers] Unzipping ${HEADERS_VERSION}"
    unzip -n headers.zip -d headers
fi

# ----- Compile

echo "=> [vulkan/Headers] Compiling ${HEADERS_VERSION}"

cd headers/*
HEADERS_FOLDER=`pwd`

mkdir -p build
cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${HEADERS_FOLDER}/build/install -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make -j 2

# ----- Install

make install
cp -R install/include/* ${ROOT_DIR}/include

#===== Vulkan Loader

cd "${ROOT_DIR}/.tmp/vulkan/${VERSION}"

# ----- Download and unzip

LOADER_VERSION=$(wget https://api.github.com/repos/KhronosGroup/Vulkan-Loader/git/refs/tags -q -O - | grep \"refs/tags/sdk-${VERSION} | tail -1 | cut -d'/' -f3 | cut -d'"' -f1)

if [ ! -f loader.zip ]; then
    echo "=> [vulkan/Loader] Downloading ${LOADER_VERSION}"
    curl -L -f0 https://github.com/KhronosGroup/Vulkan-Loader/archive/${LOADER_VERSION}.zip -o loader.zip
fi

if [ ! -d loader ]; then
    echo "=> [vulkan/Loader] Unzipping ${LOADER_VERSION}"
    unzip -n loader.zip -d loader
fi

# ----- Compile

echo "=> [vulkan/Loader] Compiling ${LOADER_VERSION}"

cd loader/*
LOADER_FOLDER=`pwd`

mkdir -p build
cd build

# Windows MinGW specific fixes
if [ `uname -o` == "Msys" ]; then
    # @note Fix needed because cfgmgr32.h in MinGW is incomplete.
    # Solution has been found in https://github.com/DeadSix27/python_cross_compile_script/issues/29
    set +e
    FIXED_MISSING_DEFINES=$(grep "#define CM_GETIDLIST_FILTER_PRESENT" ../loader/loader.c)
    set -e
    if [ -z "${FIXED_MISSING_DEFINES}" ]; then
        MISSING_DEFINES=`cat << END
#if defined(__WIN32)
#ifndef CM_GETIDLIST_FILTER_PRESENT
    #define CM_GETIDLIST_FILTER_PRESENT             (0x00000100)
#endif
#ifndef CM_GETIDLIST_FILTER_CLASS
    #define CM_GETIDLIST_FILTER_CLASS               (0x00000200)
#endif
#endif
END
`
        echo "${MISSING_DEFINES} $(cat ../loader/loader.c)" > ../loader/loader.c
    fi

    # @note This is needed because we don't have the Microsoft ASM compiler on MinGW.
    sed -i "s/CMAKE_ASM_MASM_COMPILER_WORKS/0/g" ../loader/CMakeLists.txt

    # @note This is needed the CMake file is badly done...
    set +e
    FIXED_MISSING_INCLUDE=$(grep "include_directories(\${VulkanHeaders_INCLUDE_DIR})" ../loader/CMakeLists.txt)
    set -e
    if [ -z "${FIXED_MISSING_INCLUDE}" ]; then
        echo "include_directories(\${VulkanHeaders_INCLUDE_DIR}) $(cat ../loader/CMakeLists.txt)" > ../loader/CMakeLists.txt
    fi

    # @note On Windows MinGW, still, winres.h does not exists, it is windresrc.h
    sed -i "s/winres.h/winresrc.h/g" ../loader/loader.rc

    # @note On Windows MinGW, using windres directly with paths having spaces is broken,
    # therefore we create this link first.
    # This is an ugly workaround, but I could not find anything better.
    ln -f -s "`which windres.exe`" /tmp/windres.exe
fi

# @fixme Here we have -DBUILD_WSI_WAYLAND_SUPPORT=OFF which is surely bad if one wants to use Wayland...
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${LOADER_FOLDER}/build/install -DCMAKE_RC_COMPILER=/tmp/windres.exe -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_WSI_XLIB_SUPPORT=OFF -DBUILD_WSI_WAYLAND_SUPPORT=OFF -DBUILD_TESTS=OFF -DVULKAN_HEADERS_INSTALL_DIR=${HEADERS_FOLDER}/build/install
make -j 2

# ----- Install

make install
cp -R install/lib/* ${ROOT_DIR}/lib

#===== Vulkan Validation Layers

cd "${ROOT_DIR}/.tmp/vulkan/${VERSION}"

# ----- Download and unzip

VALIDATION_LAYERS_VERSION=$(wget https://api.github.com/repos/KhronosGroup/Vulkan-ValidationLayers/git/refs/tags -q -O - | grep \"refs/tags/sdk-${VERSION} | tail -1 | cut -d'/' -f3 | cut -d'"' -f1)

if [ ! -f validation-layers.zip ]; then
    echo "=> [vulkan/ValidationLayer] Downloading ${VALIDATION_LAYERS_VERSION}"
    curl -L -f0 https://github.com/KhronosGroup/Vulkan-ValidationLayers/archive/${VALIDATION_LAYERS_VERSION}.zip -o validation-layers.zip
fi

if [ ! -d validation-layers ]; then
    echo "=> [vulkan/ValidationLayer] Unzipping ${VALIDATION_LAYERS_VERSION}"
    unzip -n validation-layers.zip -d validation-layers
fi

# ----- Compile

echo "=> [vulkan/ValidationLayers] Compiling ${VALIDATION_LAYERS_VERSION}"

# @note Hopefully shaderc has been installed before vulkan
# and thanks to alphabet, it should be the case...
# That's awesome work there...
cd validation-layers/*
VALIDATION_LAYERS_FOLDER=`pwd`
SHADERC_VERSION=$(cat "$ROOT_DIR/shaderc.lua" | grep VERSION -m 1 | cut -d '"' -f 2)
SHADERC_FOLDER=$(realpath ${ROOT_DIR}/.tmp/shaderc/${SHADERC_VERSION}/*/build/install)

mkdir -p build
cd build

# @fixme Here we have -DBUILD_WSI_WAYLAND_SUPPORT=OFF which is surely bad if one wants to use Wayland...
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${VALIDATION_LAYERS_FOLDER}/build/install -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_WSI_XLIB_SUPPORT=OFF -DBUILD_WSI_WAYLAND_SUPPORT=OFF -DBUILD_TESTS=OFF -DVULKAN_HEADERS_INSTALL_DIR=${HEADERS_FOLDER}/build/install -DGLSLANG_INSTALL_DIR=${SHADERC_FOLDER} -DSPIRV_HEADERS_INSTALL_DIR=${SHADERC_FOLDER} -DSPIRV_HEADERS_INSTALL_DIR=${SHADERC_FOLDER}
make -j 2

# ----- Install

make install
cp -R install/lib/*.so ${ROOT_DIR}/lib
cp -R install/share/* ${ROOT_DIR}/share

#===== Vulkan

echo "OK" > ${ROOT_DIR}/.tmp/vulkan/${VERSION}.txt
