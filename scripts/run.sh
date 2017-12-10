#!/bin/bash

# $1 A part of the name of a generated executable, to be run
# $2 == "debug" to enable gdb

cd $(dirname "$0")/..

# Find program indentified by the make alias
MAKE_PROGRAM="make"
if [ `uname -o` == "Msys" ]; then
    MAKE_PROGRAM="mingw32-make"
fi

# These are needed for Vulkan validation layers to work
export LD_LIBRARY_PATH=`pwd`/external/lib:${LD_LIBRARY_PATH}
export VK_LAYER_PATH=`pwd`/external/etc/explicit_layer.d

# Find a file that match the name
EXECUTABLES=$(find ./build/debug/ -type f -executable -name "*$1*" -not -name "*.so"  -not -name "*.dll")
EXECUTABLES=($EXECUTABLES)
EXECUTABLES_COUNT=${#EXECUTABLES[@]}

if [ ${EXECUTABLES_COUNT} -eq 1 ]; then
    # Build
    EXECUTABLE=${EXECUTABLES[0]}
    MAKE_TARGET=$(basename "${EXECUTABLE}" | cut -d '.' -f 1)
    
    echo -e "\e[35mBuilding ${MAKE_TARGET}...\e[39m"
    
    ${MAKE_PROGRAM} ${MAKE_TARGET}

    # Run
    if [ $? -eq 0 ]; then
        echo -e "\e[35mRunning ${EXECUTABLE}..."
        if [ "$2" == "debug" ]; then
            echo "... in debug mode."
            EXECUTABLE="gdb --quiet --directory=./external/source/vulkan/layers -ex run ${EXECUTABLE}*"
        fi
        echo -en "\e[39m"

        ${EXECUTABLE}
    else
        echo -e "\e[31m\nError during build...\e[39m"
    fi
else
    echo -en "\e[33m"
    echo "Found ${EXECUTABLES_COUNT} executables."
    for EXECUTABLE in ${EXECUTABLES[@]}; do
        echo ${EXECUTABLE}
    done
    echo -en "\e[39m"
fi
