#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "./sets/deep-deferred-g-buffer.set"
#include "./sets/deep-deferred-camera.set"
#include "./sets/deep-deferred-light.set"

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outColor;

//----- Functions

#include "./functions/deep-deferred-epiphany.sfunc"

@magma:impl:paste epiphany

//----- Program

void main()
{
    uint headerIndex = uint(gl_FragCoord.y * gBufferHeader.width + gl_FragCoord.x);

    //----- Sorting fragments by depth.
    
    GBufferNode sortedNodes[GBUFFER_MAX_NODE_DEPTH];
    uint listIndex = gBufferHeader.listIndex[headerIndex];

    uint nodeCount = 0;
    while (listIndex != 0) {
        GBufferNode node = gBufferList.nodes[listIndex];
        listIndex = node.materialId6_next26 & 0x3FFFFFF;

        // Insertion sort
        uint insertionIndex = 0;
        while (insertionIndex < nodeCount) {
            if (node.depth > sortedNodes[insertionIndex].depth) {
                break;
            }
            insertionIndex += 1;
        }

        for (uint i = nodeCount; i > insertionIndex; --i) {
            sortedNodes[i] = sortedNodes[i - 1];
        }

        sortedNodes[insertionIndex] = node;
        nodeCount += 1;
    }

    //----- Compositing.

    // Iterate over the gBuffer list, adding lights each time,
    // and composing translucent fragments.

    // @todo Allow clear color to be configurable
    vec3 color = vec3(0.2, 0.6, 0.4);
    for (uint i = 0; i < nodeCount; ++i) {
        GBufferNode node = sortedNodes[i];
        
        vec4 nodeColor = vec4(0);
        uint materialId = node.materialId6_next26 >> 26;
        switch (materialId) {
            @magma:impl:beginCases epiphany
                nodeColor = @magma:impl:call (node);
            @magma:impl:endCases
        } 
        
        color = mix(color, nodeColor.rgb, nodeColor.a);
    }

    outColor = color;
}
