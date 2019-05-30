
#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

/**
 * Inspired by from https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/prefilterenvmap.frag
 * Original by Sascha Willems under MIT license.
 */

layout(push_constant) uniform PushConstants {
	layout (offset = 0) mat4 mvp;
} pushConstants;

//----- In setup

vec2 positions[6] = vec2[](
    vec2(-1, -1),
    vec2(-1, 1),
    vec2(1, 1),
    vec2(1, 1),
    vec2(1, -1),
    vec2(-1, -1)
);

//----- Fragment forwarded out

layout (location = 0) out vec3 outUvw;

//----- Out

out gl_PerVertex {
	vec4 gl_Position;
};

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 1, 1);
	outUvw = (pushConstants.mvp * gl_Position).xyz;
}
