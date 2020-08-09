#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "./g-buffer-input.set"
#include "./g-buffer-ssbo.set"
#include "../../sets/push-constants.set"
#include "../../sets/lights.set"
#include "../../sets/shadows.set"
#include "../../sets/environment.set"

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outColor;

//----- Functions

#include "../../helpers.sfunc"
#include "../epiphany-compose.sfunc"

//----- Program

void main()
{
    setupCamera();

    // @todo Allow clear color to be configurable
    vec3 color = vec3(1.1 - length(gl_FragCoord.xy / camera.extent - vec2(0.5)));

    //----- Opaque materials

    // If depth is not zero, there are material data
    float opaqueDepth = INFINITY;
    if (gBufferRenderTargets[0].y != 0) {
        GBufferNode node;
        node.materialId6_next26 = gBufferRenderTargets[0].x;
        node.depth = uintBitsToFloat(gBufferRenderTargets[0].y);
        node.data[0] = gBufferRenderTargets[0].z;
        node.data[1] = gBufferRenderTargets[0].w;

        uint i;
        for (i = 1; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT - 1; ++i) {
            node.data[4 * i - 2] = gBufferRenderTargets[i].x;
            node.data[4 * i - 1] = gBufferRenderTargets[i].y;
            node.data[4 * i + 0] = gBufferRenderTargets[i].z;
            node.data[4 * i + 1] = gBufferRenderTargets[i].w;
        }

        // @note We make sure not to go above the data's size.
        // We cannot use a constant instead of i because otherwise,
        // the compiler can say out of bounds by itself.
        node.data[4 * i - 2] = gBufferRenderTargets[i].x;
        if (4 * i - 1 < G_BUFFER_DATA_SIZE) {
            node.data[4 * i - 1] = gBufferRenderTargets[i].y;
            if (4 * i + 0 < G_BUFFER_DATA_SIZE) {
                node.data[4 * i + 0] = gBufferRenderTargets[i].z;
                if (4 * i + 1 < G_BUFFER_DATA_SIZE) {
                    node.data[4 * i + 1] = gBufferRenderTargets[i].w;
                }
            }
        }

        opaqueDepth = node.depth;

        GBufferData gBufferData;
        gBufferData.data = node.data;
        color = composeEpiphany(node.materialId6_next26 >> 26, gBufferData, opaqueDepth).rgb;
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

        GBufferData gBufferData;
        gBufferData.data = node.data;
        vec4 nodeColor = composeEpiphany(node.materialId6_next26 >> 26, gBufferData, node.depth);

        color = mix(color, nodeColor.rgb, nodeColor.a);
    }

    outColor = color;
}
