#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform TransformsUbo {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 wEyePosition;                            
} transforms;

layout(location = 0) in vec3 inMPosition;           
layout(location = 1) in vec3 inMNormal;              
layout(location = 2) in vec3 inColor; // @todo Who cares?
layout(location = 3) in vec2 inUv;
layout(location = 4) in vec4 inMTangent;             

layout(location = 0) out vec3 outTPosition;         
layout(location = 2) out vec2 outUv;

// Lights
layout(location = 3) out vec3 outTEyePosition;      
layout(location = 4) out vec3 outTLightPosition;  

out gl_PerVertex {
    vec4 gl_Position;
};

// @todo Pass this through
vec3 lightPosition = vec3(5, 5, 0);

void main() {
    mat4 VM4 = transforms.view * transforms.model;
    mat3 VM3 = mat3(VM4);

    vec4 vPosition = VM4 * vec4(inMPosition, 1);
    gl_Position = transforms.projection * vPosition;

    // Lights
    vec3 vNormal = normalize(VM3 * inMNormal);
    vec3 vTangent = normalize(VM3 * inMTangent.xyz);
    vTangent = normalize(vTangent - vNormal * dot(vNormal, vTangent)); // Orthogonalization
    vec3 vBitangent = normalize(cross(vNormal, vTangent) * inMTangent.w);
    mat3 tbn = transpose(mat3(vTangent, vBitangent, vNormal));

    outTPosition = tbn * vPosition.xyz;
    outTEyePosition = tbn * (transforms.view * vec4(transforms.wEyePosition, 1)).xyz;
    outTLightPosition = tbn * (transforms.view * vec4(lightPosition, 1)).xyz;
    outUv = inUv;
}
