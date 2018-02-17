#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

layout(early_fragment_tests) in;

#include "./sets/deep-deferred-g-buffer.set"
#include "./sets/deep-deferred-material.set"

//----- Fragment forwarded in

layout(location = 0) in mat3 inTbn;
layout(location = 3) in vec2 inUv;

//----- Functions

#include "./functions/deep-deferred-geometry.sfunc"

@magma:impl:paste geometry

//----- Program

void main()
{
    uint headerIndex = uint(gl_FragCoord.y * gBufferHeader.width + gl_FragCoord.x);

    // Encoding materialId and next
    GBufferNode node;
    node.depth = gl_FragCoord.z;

    uint materialId = material.id;
    switch (materialId) {
        @magma:impl:beginCases geometry
            @magma:impl:call (node);
        @magma:impl:endCases
    }

    // Get the new node only if we have not been discarded
    uint listIndex = atomicAdd(gBufferList.counter, 1);
    uint oldListIndex = atomicExchange(gBufferHeader.listIndex[headerIndex], listIndex);

    node.materialId6_next26 = (materialId << 26) + oldListIndex;
    gBufferList.nodes[listIndex] = node;
}
