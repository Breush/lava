#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

// @note Don't use early depth test,
// otherwise transparent objects rendering will be erroneous.

#include "./sets/deep-deferred-g-buffer-ssbo.set"
#include "./sets/deep-deferred-light.set"
#include "./sets/deep-deferred-camera.set"
#include "./sets/deep-deferred-geometry-material.set"

//----- Fragment forwarded in

layout(location = 0) in mat3 inTbn;
layout(location = 3) in vec2 inUv;

layout (location = 0) out uvec4 outGBufferNodes[3];

//----- Functions

#include "./functions/defines.sfunc"
#include "./functions/deep-deferred-common.sfunc"
#include "./functions/deep-deferred-geometry-compose.sfunc"
#include "./functions/deep-deferred-epiphany-compose.sfunc"

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
        outGBufferNodes[0].x = node.materialId6_next26;
        outGBufferNodes[0].y = floatBitsToUint(node.depth);
        outGBufferNodes[0].z = node.materialData[0];
        outGBufferNodes[0].w = node.materialData[1];
        
        outGBufferNodes[1].x = node.materialData[2];
        outGBufferNodes[1].y = node.materialData[3];
        outGBufferNodes[1].z = node.materialData[4];
        outGBufferNodes[1].w = node.materialData[5];

        outGBufferNodes[2].x = node.materialData[6];
        outGBufferNodes[2].y = node.materialData[7];
        outGBufferNodes[2].z = node.materialData[8];
    } else {
        // If the material is translucent, we add it to the linked list,
        // so we keep all of them and they will be sorted downstream in epiphany
        uint listIndex = atomicAdd(gBufferList.counter, 1);
        uint headerIndex = uint(gl_FragCoord.y) * gBufferHeader.width + uint(gl_FragCoord.x);
        uint oldListIndex = atomicExchange(gBufferHeader.listIndex[headerIndex], listIndex);
        node.materialId6_next26 += oldListIndex;

        GBufferColorNode colorNode;
        colorNode.materialId6_next26 = node.materialId6_next26;
        colorNode.depth = node.depth;
        colorNode.color = composeEpiphany(node);
        gBufferList.nodes[listIndex] = colorNode;

        // We discard so that outGBufferHeaderListIndex will not be written
        discard;
    }
}
