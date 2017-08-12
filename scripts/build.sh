#!/bin/bash

cd $(dirname "$0")/..

#===== Prologue

# Find program indentified by the make alias
MAKE_PROGRAM="make"
if [ `uname -o` == "Msys" ]; then
    MAKE_PROGRAM="mingw32-make"
fi

#===== Build

${MAKE_PROGRAM} -j 2
