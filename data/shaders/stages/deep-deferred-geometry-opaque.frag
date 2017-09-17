#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

#include "./sets/deep-deferred-g-buffer.set"
#include "./sets/deep-deferred-material.set"

//----- Fragment forwarded in

layout(location = 0) in mat3 inTbn;
layout(location = 3) in vec2 inUv;

#include "./modules/deep-deferred-geometry.smod"

//----- Program

void main()
{
    uint headerIndex = uint(gl_FragCoord.y * gBufferHeader.width + gl_FragCoord.x);
    uint listIndex = gBufferHeader.data[headerIndex];

    if (listIndex == 0) {
        listIndex = atomicAdd(gBufferList.counter, 1);
        atomicExchange(gBufferHeader.data[headerIndex], listIndex);
    }

    setGBufferNodeFromMaterial(listIndex, 0);
}
