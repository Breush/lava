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
    // Encoding materialId and next
    // Keep next to 0 because there's nothing to link
    GBufferNode node;
    node.depth = gl_FragCoord.z;

    uint materialId = material.id;
    switch (materialId) {
        @magma:impl:beginCases geometry
            @magma:impl:call (node);
        @magma:impl:endCases
    }

    // As the fragment could have been discarded, we allocate the node only now
    uint headerIndex = uint(gl_FragCoord.y * gBufferHeader.width + gl_FragCoord.x);
    uint listIndex = gBufferHeader.listIndex[headerIndex];
    if (listIndex == 0) {
        listIndex = atomicAdd(gBufferList.counter, 1);
        atomicExchange(gBufferHeader.listIndex[headerIndex], listIndex);
    }

    node.materialId6_next26 = materialId << 26;
    gBufferList.nodes[listIndex] = node;
}
