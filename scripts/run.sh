#!/bin/bash

# $1 A part of the name of a generated executable, to be run
# $2 == "debug" to enable gdb

cd $(dirname "$0")/..

# Find program indentified by the make alias
MAKE="make"
if [ `uname -o` == "Msys" ]; then
    MAKE="mingw32-make"
fi

# These are needed for Vulkan validation layers to work
export LD_LIBRARY_PATH=`pwd`/external/lib:${LD_LIBRARY_PATH}
export VK_LAYER_PATH=`pwd`/external/etc/vulkan/explicit_layer.d

echo -e "\e[35mSetting up dependencies...\e[39m"
if [ "$2" == "profile" ]; then
    ./scripts/setup.sh --profile=true
else
    ./scripts/setup.sh
fi

# Find a make target that match the name
TARGETS=$(${MAKE} help | grep -P '^   (?!all|clean|lava-)' | grep "$1")
TARGETS=($TARGETS)
TARGETS_COUNT=${#TARGETS[@]}

CONFIG="fast-compile"
if [ "$2" != "" ]; then
    CONFIG="$2"
fi

if [ ${TARGETS_COUNT} -eq 1 ]; then
    # Build
    TARGET=${TARGETS[0]}
    EXECUTABLE="./build/${CONFIG}/${TARGET}"

    echo -e "\e[35mBuilding ${TARGET}...\e[39m"

    ${MAKE} config=${CONFIG} -j 3 ${TARGET}

    # Run
    if [ $? -eq 0 ]; then
        echo -e "\e[35mRunning ${EXECUTABLE}..."
        if [ "$2" == "debug" ]; then
            echo "... in debug mode."
            EXECUTABLE="gdb --quiet -ex run ${EXECUTABLE}*"
        elif [ "$2" == "profile" ]; then
            echo "... in profile mode."
        fi
        echo -en "\e[39m"

        ${EXECUTABLE}

        if [ "$2" == "profile" ]; then
            mkdir -p "./build/profiling"
            mv *.prof "./build/profiling/"

            if [ $? -eq 0 ]; then
                PROFILER_FILE=$(ls -t ./build/profiling/*.prof | head -1)
                ./external/bin/profiler_gui "${PROFILER_FILE}"
            else
                echo -e "\e[31m\nLooks like no .prof file has been generated.\e[39m"
            fi
        fi
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
