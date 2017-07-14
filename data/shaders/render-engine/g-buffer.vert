#version 450
#extension GL_ARB_separate_shader_objects : enable

//----- Set 0 - View

layout(set = 0, binding = 0) uniform CameraUbo {
    mat4 viewTransform;
    mat4 projectionTransform;
} camera;

//----- Set 1 - Material

//----- Set 2 - Mesh

layout(set = 2, binding = 0) uniform MeshUbo {
    mat4 transform;
} mesh;

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
    vec3 vNormal = normalize(M3 * inMNormal);
    vec3 vTangent = normalize(M3 * inMTangent.xyz);
    vTangent = normalize(vTangent - vNormal * dot(vNormal, vTangent)); // Orthogonalization
    vec3 vBitangent = normalize(cross(vNormal, vTangent) * inMTangent.w);

    outTbn = mat3(vTangent, vBitangent, vNormal);
    outUv = inUv;
}
