#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

#lava:include "./sets/deep-deferred-g-buffer.set"
#lava:include "./sets/deep-deferred-material.set"

//----- Fragment forwarded in

layout(location = 0) in mat3 inTbn;
layout(location = 3) in vec2 inUv;

#lava:include "./modules/deep-deferred-geometry.smod"

//----- Program

void main()
{
    uint headerIndex = uint(gl_FragCoord.y * gBufferHeader.width + gl_FragCoord.x);
    uint listIndex = atomicAdd(gBufferList.counter, 1);
    uint oldListIndex = atomicExchange(gBufferHeader.data[headerIndex], listIndex);

    setGBufferNodeFromMaterial(listIndex, oldListIndex);
}
