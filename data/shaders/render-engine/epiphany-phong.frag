#version 450
#extension GL_ARB_separate_shader_objects : enable

// @todo Add world position (or just depth?)
vec3 wPosition = vec3(1, 0, 0);
layout(set = 0, binding = 0) uniform sampler2D normalSampler;
layout(set = 0, binding = 1) uniform sampler2D albedoSampler;

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outColor;

//----- Program

// @todo Get the lights and such from the program, via an LLL
vec3 wEyePosition = vec3(5, 5, 5);
vec3 wLightPosition = vec3(3, -3, 0);

void main()
{
    vec3 albedo = texture(albedoSampler, inUv).xyz;
    vec3 normal = 2 * texture(normalSampler, inUv).xyz - 1;

    // Material-specific... get them from ORM somehow
    float ka = 0.2;
    float kd = 0.7;
    float ks = 0.2;
    float alpha = 10;

    // For each light
    float ia = 1;
    float id = 1;
    float is = 1;

    vec3 l = normalize(wLightPosition - wPosition);
    vec3 v = normalize(wEyePosition - wPosition);
    float cosTheta = dot(normal, l);
    vec3 r = normalize(2 * cosTheta * normal - l);
    float cosOmega = dot(r, v);

    vec3 ambient = ia * ka * albedo;
    vec3 diffuse = id * kd * cosTheta * albedo;
    vec3 specular = vec3(is * ks * pow(cosOmega, alpha));

    outColor = ambient + diffuse + specular;
}
