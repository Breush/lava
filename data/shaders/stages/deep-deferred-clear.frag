#version 450
#extension GL_ARB_separate_shader_objects : enable

#lava:include "./sets/deep-deferred-g-buffer.set"

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Program

void main()
{
    // If GBufferNode.next is 0, it means the end of the list.
    // So we start this count at 1.
    gBufferList.counter = 1;

    uint headerIndex = uint(gl_FragCoord.y * gBufferHeader.width + gl_FragCoord.x);
    gBufferHeader.data[headerIndex] = 0;
}
