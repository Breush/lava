#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "./sets/deep-deferred-g-buffer.set"

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Program

void main()
{
    gBufferList.counter = 0;

    uint headerIndex = uint(gl_FragCoord.y * gBufferHeader.width + gl_FragCoord.x);
    gBufferHeader.listIndex[headerIndex] = 0;
}
