#!/bin/bash

cd $(dirname "$0")/..

git submodule init
git submodule update
git update-index --assume-unchanged .setup.json
premake5 gmake $*
