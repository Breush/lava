#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUv;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragUv = inUv;
}
