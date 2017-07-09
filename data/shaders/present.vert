#version 450
#extension GL_ARB_separate_shader_objects : enable

//----- Vertex data in

layout(location = 0) in vec2 inPosition;

//----- Out

out gl_PerVertex {
    vec4 gl_Position;
};

//----- Program

void main() {
    gl_Position = vec4(inPosition, 0, 1);
}
