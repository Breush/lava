#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "./sets/deep-deferred-g-buffer-input.set"
#include "./sets/deep-deferred-g-buffer-ssbo.set"
#include "./sets/deep-deferred-light.set"
#include "./sets/deep-deferred-camera.set"

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outColor;

//----- Functions

#include "./functions/defines.sfunc"
#include "./functions/deep-deferred-common.sfunc"
#include "./functions/deep-deferred-epiphany-compose.sfunc"

//----- Program

void main()
{
    // @todo Allow clear color to be configurable
    vec3 color = vec3(0.2, 0.6, 0.4);

    //----- Opaque materials

    // If depth is not zero, there are material data
    float opaqueDepth = INFINITY;
    if (gBufferNode0.y != 0) {
        GBufferNode node;
        node.materialId6_next26 = gBufferNode0.x;
        node.depth = uintBitsToFloat(gBufferNode0.y);
        node.materialData[0] = gBufferNode0.z;
        node.materialData[1] = gBufferNode0.w;

        node.materialData[2] = gBufferNode1.x;
        node.materialData[3] = gBufferNode1.y;
        node.materialData[4] = gBufferNode1.z;
        node.materialData[5] = gBufferNode1.w;

        node.materialData[6] = gBufferNode2.x;
        node.materialData[7] = gBufferNode2.y;
        node.materialData[8] = gBufferNode2.z;

        opaqueDepth = node.depth;
        color = composeEpiphany(node).rgb;
    }

    //----- Translucent materials

    // Sort fragments by depth
    uint sortedListIndices[GBUFFER_MAX_NODE_DEPTH];
    uint headerIndex = uint(gl_FragCoord.y) * gBufferHeader.width + uint(gl_FragCoord.x);
    uint listIndex = gBufferHeader.listIndex[headerIndex];

    uint nodeCount = 0;
    while (listIndex != 0 && nodeCount < GBUFFER_MAX_NODE_DEPTH) {
        GBufferColorNode node = gBufferList.nodes[listIndex];
        if (node.depth > opaqueDepth) continue;

        // Insertion sort
        uint insertionIndex = 0;
        while (insertionIndex < nodeCount) {
            const uint sortedListIndex = sortedListIndices[insertionIndex]; 
            if (node.depth > gBufferList.nodes[sortedListIndex].depth) {
                break;
            }
            insertionIndex += 1;
        }

        for (uint i = nodeCount; i > insertionIndex; --i) {
            sortedListIndices[i] = sortedListIndices[i - 1];
        }

        sortedListIndices[insertionIndex] = listIndex;
        listIndex = node.materialId6_next26 & 0x3FFFFFF;
        nodeCount += 1;
    }

    // Iterate over the gBuffer list, adding lights each time,
    // and composing translucent fragments.
    for (uint i = 0; i < nodeCount; ++i) {
        GBufferColorNode node = gBufferList.nodes[sortedListIndices[i]];
        color = mix(color, node.color.rgb, node.color.a);
    }

    outColor = color;
}
