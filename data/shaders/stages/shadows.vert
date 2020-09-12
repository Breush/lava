#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

#include "./sets/push-constants.set"
#include "./sets/mesh.set"

//----- Out

out gl_PerVertex {
    vec4 gl_Position;
};

//----- Program

void main() {
    setupMesh();

    gl_Position = pushConstants.cascadeTransform * mesh.transform * vec4(inMPosition, 1);
}
