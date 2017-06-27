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
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec3 fragWorldPosition;
layout(location = 1) out mat3 fragTbn; // @fixme Better compute the lights transformations in the vertex shader
layout(location = 4) out vec3 fragColor;
layout(location = 5) out vec2 fragUv;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 worldPosition = transforms.model * vec4(inPosition, 1);

    gl_Position = transforms.projection * transforms.view * worldPosition;

    fragWorldPosition = worldPosition.xyz;
    fragColor = inColor;
    fragUv = inUv;

    vec3 bitangent = cross(inNormal, inTangent.xyz) * inTangent.w;
    fragTbn = mat3(transforms.model) * mat3(inTangent.xyz, bitangent, inNormal);
}
