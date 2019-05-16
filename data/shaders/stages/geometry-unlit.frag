#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

// Early depth test, as everything is sorted.
layout(early_fragment_tests) in;

//----- Out data

layout(location = 0) out vec4 outColor;

//----- Program

void main()
{
    const float nearWhite = 0.8;
    const float farBlack = 0.99;
    const float intensity = (farBlack - gl_FragCoord.z) / (farBlack - nearWhite);
    outColor = vec4(vec3(1), intensity);
}
