#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

#include "./sets/push-constants.set"
#include "./sets/lights.set"

//----- Vertex data in

layout(location = 0) in vec3 inMPosition;

//----- Out

out gl_PerVertex {
    vec4 gl_Position;
};

//----- Program

void main() {
    setupMesh();

    // @note This light will become lights[0] one day, when we handle
    // multiple runtime lights.

    gl_Position = light.transform * mesh.transform * vec4(inMPosition, 1);
}
