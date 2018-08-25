#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

// @note Don't use early depth test,
// otherwise transparent objects rendering will be erroneous.

#include "./g-buffer-ssbo.set"
#include "../../sets/camera.set"
#include "../../sets/material.set"

//----- Fragment forwarded in

layout(location = 0) in mat3 inTbn;
layout(location = 3) in vec2 inUv;

layout (location = 0) out uvec4 outGBufferRenderTargets[DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT];

//----- Functions

#include "../../helpers.sfunc"
#include "./geometry-compose.sfunc"

//----- Program

void main()
{
    uint materialId = material.id;

    // Encoding materialId and next
    GBufferNode node;
    node.materialId6_next26 = materialId << 26;
    node.depth = gl_FragCoord.z;

    GBufferData gBufferData;
    bool translucent = composeGeometry(materialId, gBufferData, node.depth);
    node.data = gBufferData.data;

    if (!translucent) {
        // If the material is opaque, we just output to the render targets,
        // as the depth resolution will occur further in the pipeline

        // Storing the node in the render targets
        outGBufferRenderTargets[0].x = node.materialId6_next26;
        outGBufferRenderTargets[0].y = floatBitsToUint(node.depth);
        outGBufferRenderTargets[0].z = node.data[0];
        outGBufferRenderTargets[0].w = node.data[1];

        // @note We might write uninitialized memory if we go over data's size,
        // but checking would cost us more in terms of performences.
        for (uint i = 1; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
            outGBufferRenderTargets[i].x = node.data[4 * i - 2];
            outGBufferRenderTargets[i].y = node.data[4 * i - 1];
            outGBufferRenderTargets[i].z = node.data[4 * i + 0];
            outGBufferRenderTargets[i].w = node.data[4 * i + 1];
        }
    } else {
        // If the material is translucent, we add it to the linked list,
        // so we keep all of them and they will be sorted downstream in epiphany
        uint listIndex = atomicAdd(gBufferList.counter, 1);
        uint headerIndex = uint(gl_FragCoord.y) * gBufferHeader.width + uint(gl_FragCoord.x);
        uint oldListIndex = atomicExchange(gBufferHeader.listIndex[headerIndex], listIndex);
        node.materialId6_next26 += oldListIndex;

        gBufferList.nodes[listIndex] = node;

        // We discard so that outGBufferHeaderListIndex will not be written
        discard;
    }
}
