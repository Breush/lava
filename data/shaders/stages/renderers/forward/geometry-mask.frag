#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

// No early depth test, as everything some fragment might be discarded.
// layout(early_fragment_tests) in;

#include "../../sets/push-constants.set"
#include "../../sets/material.set"
#include "../../sets/material-global.set"
#include "../../sets/lights.set"
#include "../../sets/shadows.set"
#include "../../sets/environment.set"

//----- Fragment forwarded in

layout(location = 0) in mat3 inTbn;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec3 inCubeUvw;

//----- Out data

layout(location = 0) out vec4 outColor;

//----- Functions

#include "../../helpers.sfunc"
#include "../../g-buffer-data.sfunc"
#include "../geometry-compose.sfunc"
#include "../epiphany-compose.sfunc"

//----- Program

void main()
{
    setupCamera();

    GBufferData gBufferData;

    uint materialId = material.id;

    composeGeometry(materialId, gBufferData, gl_FragCoord.z);
    outColor = composeEpiphany(materialId, gBufferData, gl_FragCoord.z);

    if (outColor.a < 0.5) {
        discard;
    }
}
