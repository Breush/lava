#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraUbo {
    vec3 wPosition;
} camera;

// @todo To be replaced by LLL
layout(set = 0, binding = 1) uniform LightUbo {
    vec3 wPosition;
} light;

// @todo Add world position (or just depth?)
vec3 wPosition = vec3(1, 0, 0);
layout(set = 0, binding = 2) uniform sampler2D normalSampler;
layout(set = 0, binding = 3) uniform sampler2D albedoSampler;

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outColor;

//----- Program

void main()
{
    vec3 albedo = texture(albedoSampler, inUv).xyz;
    vec3 normal = 2 * texture(normalSampler, inUv).xyz - 1;

    // No normal > flat shading
    // (the normal should already be normalized)
    if (dot(normal, normal) <= 0.5) {
        outColor = albedo;
        return;
    }

    // Material-specific... get them from ORM somehow
    float ka = 0.2;
    float kd = 0.7;
    float ks = 0.2;
    float alpha = 2;

    // For each light
    // @note Distance should affect intensity
    float ia = 1;
    float id = 1;
    float is = 1;

    vec3 l = normalize(light.wPosition - wPosition);
    vec3 v = normalize(camera.wPosition - wPosition);
    float cosTheta = dot(normal, l);
    vec3 r = normalize(2 * cosTheta * normal - l);
    float cosOmega = dot(r, v);

    vec3 ambient = ia * ka * albedo;
    vec3 diffuse = id * kd * cosTheta * albedo;
    vec3 specular = vec3(is * ks * pow(cosOmega, alpha));

    outColor = ambient + diffuse + specular;
}
