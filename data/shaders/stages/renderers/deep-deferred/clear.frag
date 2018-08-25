#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "./g-buffer-ssbo.set"

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Program

void main()
{
    gBufferList.counter = 1;

    uint headerIndex = uint(gl_FragCoord.y) * gBufferHeader.width + uint(gl_FragCoord.x);
    gBufferHeader.listIndex[headerIndex] = 0;
}
