#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D baseColorSampler;
layout(binding = 2) uniform sampler2D metallicRoughnessSampler;

layout(location = 0) in vec3 fragWorldPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

// Pass this through
struct PointLight {
    vec3 position;
};
PointLight pointLight = PointLight(vec3(-5, 5, -10));


vec4 lightContribution();
vec4 pointLightContribution(PointLight pointLight, vec3 normal);

void main()
{
    vec4 lightColor = lightContribution();
    vec4 textureColor = vec4(fragColor * texture(baseColorSampler, fragUv).rgb, 1.0);
    textureColor = vec4(fragColor * texture(metallicRoughnessSampler, fragUv).rgb, 1.0);

    // @fixme Have a way to enable/disable uvs
    outColor = lightColor * textureColor;
}

vec4 lightContribution()
{
    vec4 ambientColor = vec4(0.2, 0.2, 0.2, 1.0);
    vec4 pointLightColor = pointLightContribution(pointLight, fragNormal);

    return ambientColor + pointLightColor;
}

vec4 lightContribution(vec3 lightDirection, vec3 normal)
{
    float diffuseFactor = dot(normal, lightDirection);
    vec4 diffuseColor = vec4(diffuseFactor, diffuseFactor, diffuseFactor, 1.0);
    return diffuseColor;
}

vec4 pointLightContribution(PointLight pointLight, vec3 normal)
{
    vec3 lightDirection = fragWorldPosition - pointLight.position;
    float lightDistance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    vec4 contribution = lightContribution(lightDirection, normal);
    
    // @todo Have more complex attenuation setup
    float attenuation = 0.3 * lightDistance;

    return contribution / attenuation;
}
