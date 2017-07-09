#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D sourceSampler;

//----- Out data

layout(location = 0) out vec3 outPresent;

//----- Program

void main()
{
    // @todo Use sampler
    outPresent = vec3(gl_FragCoord.xy, 1);
}
