#!/bin/bash

cd $(dirname "$0")/..

#===== Prologue

# Find program indentified by the make alias
MAKE="make"
if [ `uname -o` == "Msys" ]; then
    MAKE="mingw32-make"
fi

#===== Build

${MAKE} -j 2
