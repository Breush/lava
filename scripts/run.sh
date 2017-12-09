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
    EXECUTABLES=$(find ./build/debug/ -type f -executable -name "*$1*" -not -name "*.*")
    EXECUTABLES=($EXECUTABLES)
    EXECUTABLES_COUNT=${#EXECUTABLES[@]}

    if [ ${EXECUTABLES_COUNT} -eq 1 ]; then
        EXECUTABLE=${EXECUTABLES[0]}
        echo -en "\e[33m"
        echo "Running ${EXECUTABLE}..."

        if [ "$2" == "debug" ]; then
            echo "... in debug mode."
            EXECUTABLE="gdb --quiet --directory=./external/source/vulkan/layers -ex run ${EXECUTABLE}*"
        fi
        echo -en "\e[39m"

        ${EXECUTABLE}
    else
        echo -en "\e[33m"
        echo "Found ${EXECUTABLES_COUNT} executables."
        for EXECUTABLE in ${EXECUTABLES[@]}; do
            echo ${EXECUTABLE}
        done
        echo -en "\e[39m"
    fi
else
    echo -e "\e[31m\nError during setup or build...\e[39m"
fi
