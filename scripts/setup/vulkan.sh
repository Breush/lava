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

if [ ! -f headers.zip ]; then
    curl -L https://github.com/KhronosGroup/Vulkan-Headers/archive/sdk-${VERSION}.zip -o headers.zip
fi

if [ ! -d headers ]; then
    unzip -n headers.zip -d headers
fi

# ----- Compile

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

if [ ! -f loader.zip ]; then
    curl -L https://github.com/KhronosGroup/Vulkan-Loader/archive/sdk-${VERSION}.zip -o loader.zip
fi

if [ ! -d loader ]; then
    unzip -n loader.zip -d loader
fi

# ----- Compile

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
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${LOADER_FOLDER}/build/install -DCMAKE_RC_COMPILER=/tmp/windres.exe -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_WSI_XLIB_SUPPORT=OFF -DBUILD_WSI_WAYLAND_SUPPORT=OFF -DBUILD_TESTS=OFF -DVulkanHeaders_INCLUDE_DIR=${HEADERS_FOLDER}/build/install/include -DVulkanRegistry_DIR=${HEADERS_FOLDER}/build/install/share/vulkan/registry
make -j 2

# ----- Install

make install
cp -R install/lib/* ${ROOT_DIR}/lib

#===== Vulkan

echo "OK" > ${ROOT_DIR}/.tmp/vulkan/${VERSION}.txt

# @todo Add validation layers back!
