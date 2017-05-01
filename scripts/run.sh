#!/bin/bash

cd $(dirname "$0")/..

# These are needed for Vulkan validation layers to work
export LD_LIBRARY_PATH=`pwd`/external/lib:${LD_LIBRARY_PATH}
export VK_LAYER_PATH=`pwd`/external/etc/explicit_layer.d

make -j 2 && ./build/debug/app
