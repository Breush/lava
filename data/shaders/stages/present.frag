#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D sourceSampler;

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outPresent;

//----- Program

void main()
{
    outPresent = texture(sourceSampler, inUv).xyz;
}
