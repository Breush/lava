#!/bin/bash

cd $(dirname "$0")/..

STYLE=`cat .vscode/settings.json | grep "C_Cpp.clang_format_style" | cut -d'"' -f4`
SOURCE_FILES=`find source -name *.cpp`
HEADER_FILES=`find include -name *.hpp`
INLINE_FILES=`find include -name *.inl`

clang-format -i -style="$STYLE" -verbose -sort-includes $SOURCE_FILES $HEADER_FILES $INCLUDE_FILES
