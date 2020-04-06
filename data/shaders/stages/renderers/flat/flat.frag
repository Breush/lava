#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#include "../../sets/material.set"

//----- In data

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec4 outColor;

//----- Functions

@magma:impl:paste flat

//----- Program

void main()
{
    vec4 color = vec4(0);

    uint materialId = material.id;
    switch (materialId) {
        @magma:impl:beginCases flat
            color = @magma:impl:call ();
        @magma:impl:endCases
    }

    outColor = color;
}
