#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUv;

layout(location = 0) out vec3 fragWorldPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragUv;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 worldPosition = ubo.view * ubo.model * vec4(inPosition, 1.0);

    gl_Position = ubo.projection * worldPosition;

    fragWorldPosition = worldPosition.xyz;
    fragNormal = inNormal;
    fragColor = inColor;
    fragUv = inUv;
}
