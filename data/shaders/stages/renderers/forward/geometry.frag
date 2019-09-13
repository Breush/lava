#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

// Early depth test, as everything is sorted.
layout(early_fragment_tests) in;

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

@magma:impl:paste geometry
@magma:impl:paste epiphany

//----- Program

void main()
{
    setupCamera();

    GBufferData gBufferData;

    uint materialId = material.id;

    // @note We don't use the boolean result of geometry,
    // as, in forward renderer, we have no way to change what we're doing.
    switch (materialId) {
        @magma:impl:beginCases geometry
            @magma:impl:call (gBufferData, gl_FragCoord.z);
        @magma:impl:endCases
    }

    vec4 color;
    switch (materialId) {
        @magma:impl:beginCases epiphany
            color = @magma:impl:call (gBufferData, gl_FragCoord.z);
        @magma:impl:endCases
    }

    outColor = color;
}
