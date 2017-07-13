#!/bin/bash

# $1 A part of the name of a generated executable, to be run
# $2 == "debug" to enable gdb

cd $(dirname "$0")/..

# These are needed for Vulkan validation layers to work
export LD_LIBRARY_PATH=`pwd`/external/lib:${LD_LIBRARY_PATH}
export VK_LAYER_PATH=`pwd`/external/etc/explicit_layer.d

./scripts/setup.sh && ./scripts/build.sh

if [ $? -eq 0 ]; then
    # Find a file that match the name
    EXECUTABLE=$(find ./build/debug/ -type f -executable -name "*$1*" ! -name "*.*")
    echo "Running ${EXECUTABLE}..."

    if [ "$2" == "debug" ]; then
        echo "... in debug mode."
        EXECUTABLE="gdb ${EXECUTABLE} --quiet --directory=./external/source/vulkan/layers"
    fi

    ${EXECUTABLE}
else
    echo "Error during setup or build..."
fi
