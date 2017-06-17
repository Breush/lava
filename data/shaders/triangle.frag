#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D tex;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in float depth;

layout(location = 0) out vec4 outColor;

void main()
{
    // @fixme Have a way to unable or not uvs
    outColor = vec4(fragColor/* * texture(tex, fragUv).rgb*/, 1.0);
    outColor = vec4(1. - depth / 4., 0.0, 0.0, 1.0);
}
