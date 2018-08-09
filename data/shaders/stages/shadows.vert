#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

#include "./sets/shadows-light.set"
#include "./sets/mesh.set"

//----- Vertex data in

// @fixme Do a renderShadows() in mesh - just passing the position
layout(location = 0) in vec3 inMPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inMNormal;
layout(location = 3) in vec4 inMTangent;

//----- Out

out gl_PerVertex {
    vec4 gl_Position;
};

//----- Program

void main() {
    gl_Position = light.transform * mesh.transform * vec4(inMPosition, 1);
}
