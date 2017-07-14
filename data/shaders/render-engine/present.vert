#version 450
#extension GL_ARB_separate_shader_objects : enable

//----- In setup

vec2 positions[6] = vec2[](
    vec2(-1, -1),
    vec2(-1, 1),
    vec2(1, 1),
    vec2(1, 1),
    vec2(1, -1),
    vec2(-1, -1)
);

vec2 uvs[6] = vec2[](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 1),
    vec2(1, 1),
    vec2(1, 0),
    vec2(0, 0)
);

//----- Fragment forwarded out

layout(location = 0) out vec2 outUv;

//----- Out

out gl_PerVertex {
    vec4 gl_Position;
};


//----- Program

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0, 1);

    outUv = uvs[gl_VertexIndex];
}
