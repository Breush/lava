#version 450
#extension GL_ARB_separate_shader_objects : enable

//----- Set 0 - View

layout(set = 0, binding = 0) uniform CameraUbo {
    mat4 viewTransform;
    mat4 projectionTransform;
    vec4 wPosition;
    vec4 wPointLightPosition; // @todo This should be a push_constant thing
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

layout(location = 0) out vec3 outTPosition;
layout(location = 1) out vec2 outUv;
layout(location = 2) out vec3 outTEyePosition;      
layout(location = 3) out vec3 outTLightPosition;  

//----- Out

out gl_PerVertex {
    vec4 gl_Position;
};

//----- Program

void main() {
    mat4 VM4 = camera.viewTransform * mesh.transform;
    mat3 VM3 = mat3(VM4);

    vec4 vPosition = VM4 * vec4(inMPosition, 1);
    gl_Position = camera.projectionTransform * vPosition;

    // Lights
    vec3 vNormal = normalize(VM3 * inMNormal);
    vec3 vTangent = normalize(VM3 * inMTangent.xyz);
    vTangent = normalize(vTangent - vNormal * dot(vNormal, vTangent)); // Orthogonalization
    vec3 vBitangent = normalize(cross(vNormal, vTangent) * inMTangent.w);
    mat3 tbn = transpose(mat3(vTangent, vBitangent, vNormal));

    outTPosition = tbn * vPosition.xyz;
    outTEyePosition = tbn * (camera.viewTransform * camera.wPosition).xyz;
    outTLightPosition = tbn * (camera.viewTransform * camera.wPointLightPosition).xyz;
    outUv = inUv;
}
