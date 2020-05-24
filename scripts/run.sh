#!/bin/bash

set -e

# $1 A part of the name of a generated executable, to be run
# $2 == "debug" to enable gdb

cd $(dirname "$0")/..

if [ "${LAVA_INFO_LOG_LEVEL}" == "" ]; then
    export LAVA_INFO_LOG_LEVEL=2
fi

# These are needed for Vulkan validation layers to work
export LD_LIBRARY_PATH=`pwd`/external/lib:${LD_LIBRARY_PATH}
export VK_LAYER_PATH=`pwd`/external/etc/vulkan/explicit_layer.d

echo -e "\e[35mSetting up dependencies...\e[39m"
./scripts/setup.sh

# Find a make target that match the name
TARGETS=$(make help | grep -P '^   (?!all|clean|lava-)' | grep "$1")
TARGETS=($TARGETS)
TARGETS_COUNT=${#TARGETS[@]}

CONFIG="fast-compile"
if [ "$2" != "" ] && [ "$2" != "slow" ]; then
    CONFIG="$2"
fi

if [ ${TARGETS_COUNT} -eq 1 ]; then
    # Build
    TARGET=${TARGETS[0]}
    EXECUTABLE="./build/${CONFIG}/${TARGET}"

    echo -e "\e[35mBuilding ${TARGET}...\e[39m"

    if [ "$2" == "slow" ]; then
        make config=${CONFIG} ${TARGET}
    else
        make config=${CONFIG} -j 6 ${TARGET}
    fi

    # Run
    if [ $? -eq 0 ]; then
        echo -e "\e[35mRunning ${EXECUTABLE}..."
        if [ "$2" == "debug" ]; then
            echo "... in debug mode."
            EXECUTABLE="gdb --quiet -ex run ${EXECUTABLE}*"
        fi
        echo -en "\e[39m"

        ${EXECUTABLE}
    else
        echo -e "\e[31m\nError during build...\e[39m"
    fi
else
    echo -en "\e[33m"
    echo "Found ${TARGETS_COUNT} targets matching '$1'."
    for EXECUTABLE in ${TARGETS[@]}; do
        echo ${EXECUTABLE}
    done
    echo -en "\e[39m"
fi
