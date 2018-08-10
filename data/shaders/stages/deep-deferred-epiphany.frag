#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "./sets/deep-deferred-g-buffer-input.set"
#include "./sets/deep-deferred-g-buffer-ssbo.set"
#include "./sets/camera.set"
#include "./sets/deep-deferred-epiphany-lights.set"

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

        // @fixme Can't this be an array,
        // making it easier to factorize?
        node.materialData[2] = gBufferNode1.x;
        node.materialData[3] = gBufferNode1.y;
        node.materialData[4] = gBufferNode1.z;
        node.materialData[5] = gBufferNode1.w;

        node.materialData[6] = gBufferNode2.x;
        node.materialData[7] = gBufferNode2.y;
        node.materialData[8] = gBufferNode2.z;
        node.materialData[9] = gBufferNode2.w;

        node.materialData[10] = gBufferNode3.x;
        node.materialData[11] = gBufferNode3.y;

        opaqueDepth = node.depth;

        color = composeEpiphany(node).rgb;
    }

    //----- Translucent materials

    // Sort fragments by depth
    uint sortedListIndices[DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH];
    uint headerIndex = uint(gl_FragCoord.y) * gBufferHeader.width + uint(gl_FragCoord.x);
    uint listIndex = gBufferHeader.listIndex[headerIndex];

    uint nodeCount = 0;
    while (listIndex != 0 && nodeCount < DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH) {
        GBufferNode node = gBufferList.nodes[listIndex];

        // Do not count translucent fragments that are behind opaque ones.
        if (node.depth <= opaqueDepth) {
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
            nodeCount += 1;
        }

        listIndex = node.materialId6_next26 & 0x3FFFFFF;
    }

    // Iterate over the gBuffer list, adding lights each time,
    // and composing translucent fragments.
    for (uint i = 0; i < nodeCount; ++i) {
        GBufferNode node = gBufferList.nodes[sortedListIndices[i]];
        vec4 nodeColor = composeEpiphany(node);
        color = mix(color, nodeColor.rgb, nodeColor.a);
    }

    outColor = color;
}
