#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

// @note Don't use early depth test,
// otherwise transparent objects rendering will be erroneous.

#include "./sets/deep-deferred-g-buffer-ssbo.set"
#include "./sets/camera.set"
#include "./sets/material.set"

//----- Fragment forwarded in

layout(location = 0) in mat3 inTbn;
layout(location = 3) in vec2 inUv;

layout (location = 0) out uvec4 outGBufferRenderTargets[DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT];

//----- Functions

#include "./functions/defines.sfunc"
#include "./functions/deep-deferred-common.sfunc"
#include "./functions/deep-deferred-geometry-compose.sfunc"

//----- Program

void main()
{
    uint materialId = material.id;

    // Encoding materialId and next
    GBufferNode node;
    node.materialId6_next26 = materialId << 26;
    node.depth = gl_FragCoord.z;

    bool translucent = composeGeometry(node);

    if (!translucent) {
        // If the material is opaque, we just output to the render targets,
        // as the depth resolution will occur further in the pipeline

        // Storing the node in the render targets
        outGBufferRenderTargets[0].x = node.materialId6_next26;
        outGBufferRenderTargets[0].y = floatBitsToUint(node.depth);
        outGBufferRenderTargets[0].z = node.materialData[0];
        outGBufferRenderTargets[0].w = node.materialData[1];

        // @note We might write uninitialized memory if we go over materialData's size,
        // but checking would cost us more in terms of performences.
        for (uint i = 1; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
            outGBufferRenderTargets[i].x = node.materialData[4 * i - 2];
            outGBufferRenderTargets[i].y = node.materialData[4 * i - 1];
            outGBufferRenderTargets[i].z = node.materialData[4 * i + 0];
            outGBufferRenderTargets[i].w = node.materialData[4 * i + 1];
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
