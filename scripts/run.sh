#!/bin/bash

cd $(dirname "$0")/..

export VK_LAYER_PATH=`pwd`/external/etc/explicit_layer.d

make -j 2 && ./build/debug/app
