#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform TransformsUbo {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
} transforms;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor; // @todo Who cares?
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec3 fragWorldPosition; // @fixme Remove, should be 0 once in TBN space
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec2 fragUv;

// Lights
layout(location = 3) out vec3 fragEyePosition;
layout(location = 4) out vec3 fragLightPosition;

out gl_PerVertex {
    vec4 gl_Position;
};

// @todo Pass this through
vec3 lightPosition = vec3(5, 5, 0);

void main() {
    vec4 worldPosition = transforms.model * vec4(inPosition, 1);

    gl_Position = transforms.projection * transforms.view * worldPosition;

    fragColor = inColor;
    fragUv = inUv;

    // Lights
    mat4 MV = transforms.view * transforms.model;

    vec3 normal = normalize(MV * vec4(inNormal, 1)).xyz;
    vec3 tangent = normalize(MV * vec4(inTangent.xyz, 1)).xyz;
    tangent = normalize(tangent - normal * dot(normal, tangent)); // Orthogonalization
    vec3 bitangent = cross(normal, tangent) * inTangent.w;

    mat3 tbn = transpose(mat3(tangent, bitangent, normal));
    fragWorldPosition = tbn * mat3(MV) * inPosition;
    fragEyePosition = tbn * mat3(MV) * transforms.cameraPosition;
    fragLightPosition = tbn * mat3(MV) * lightPosition;
}
