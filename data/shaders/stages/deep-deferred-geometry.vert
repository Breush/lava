#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

#include "./sets/deep-deferred-camera.set"
#include "./sets/deep-deferred-mesh.set"

//----- Vertex data in

layout(location = 0) in vec3 inMPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inMNormal;
layout(location = 3) in vec4 inMTangent;

//----- Fragment forwarded out

layout(location = 0) out mat3 outTbn;
layout(location = 3) out vec2 outUv;

//----- Out

out gl_PerVertex {
    vec4 gl_Position;
};

//----- Program

void main() {
    mat3 M3 = mat3(mesh.transform);

    vec4 vPosition = camera.viewTransform * mesh.transform * vec4(inMPosition, 1);
    gl_Position = camera.projectionTransform * vPosition;

    // Tangent-space
    vec3 wNormal = normalize(inMNormal);
    vec3 wTangent = normalize(inMTangent.xyz);
    wTangent = normalize(wTangent - wNormal * dot(wNormal, wTangent)); // Orthogonalization
    vec3 wBitangent = normalize(cross(wNormal, wTangent) * inMTangent.w);

    outTbn = M3 * mat3(wTangent, wBitangent, wNormal);
    outUv = inUv;
}
