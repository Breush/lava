#!/bin/bash

cd $(dirname "$0")/..
./external/bin/glslangValidator -V ./data/shaders/triangle.vert -o ./data/shaders/triangle.vert.spv
./external/bin/glslangValidator -V ./data/shaders/triangle.frag -o ./data/shaders/triangle.frag.spv
