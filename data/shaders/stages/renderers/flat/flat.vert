#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

#include "../../sets/push-constants.set"

//----- Vertex data in

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUv;

//----- Fragment forwarded out

layout(location = 0) out vec2 outUv;

//----- Out

out gl_PerVertex {
    vec4 gl_Position;
};

//----- Program

void main() {
    setupCamera();
    setupFlat();

    outUv = inUv;

    vec3 wPosition = flatUbo.transform * vec3(inPosition, 1);
    gl_Position = vec4(wPosition.xy, 0, 1);

    // Matching top left (0, 0) to (-1, -1)
    // and bottom right  (width, height) to (1, 1)
    // to get back to vulkan conventions.
    gl_Position.x = (2. * gl_Position.x / camera.extent.x) - 1.;
    gl_Position.y = (2. * gl_Position.y / camera.extent.y) - 1.;
}
