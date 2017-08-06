#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraUbo {
    mat4 invertedView;
    mat4 invertedProjection;
    vec4 wPosition;
} camera;

// @todo To be replaced by LLL
layout(set = 0, binding = 1) uniform LightUbo {
    vec4 wPosition;
    float radius;
} light;

layout(set = 0, binding = 2) uniform sampler2D normalSampler;
layout(set = 0, binding = 3) uniform sampler2D albedoSampler;
layout(set = 0, binding = 4) uniform sampler2D ormSampler;
layout(set = 0, binding = 5) uniform sampler2D depthSampler;

//----- Fragment in

layout(location = 0) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outColor;

//----- Headers

vec3 wPositionFromDepth(float depth, vec2 coord);

//----- Program

void main()
{
    vec3 albedo = texture(albedoSampler, inUv).rgb;
    vec3 normal = 2 * texture(normalSampler, inUv).xyz - 1;
    vec3 orm = texture(ormSampler, inUv).rgb;
    float depth = texture(depthSampler, inUv).x;

    // No normal => flat shading
    // (the normal should already be normalized)
    if (dot(normal, normal) <= 0.5) {
        outColor = albedo;
        return;
    }
    
    // General ambient
    vec3 wPosition = wPositionFromDepth(depth, inUv);
    vec3 v = normalize(camera.wPosition.xyz - wPosition.xyz);
    float ambient = 0.2;

    // Material-specific
    float occlusion = orm.r;
    float roughness = orm.g;
    float metallic = orm.b;
    float ka = 1 - 0.2 * roughness;
    float kd = 0.5 + 0.2 * roughness;
    float ks = 0.25 - 0.23 * roughness;
    float alpha = pow(2, 1 + 7 * metallic);

    // For each light
    // @note Distance should affect intensity
    float id = 1;
    float is = 1;

    float diffuse = 0;
    float specular = 0;

    // Check whether the lighting should have an effect
    vec3 lightVector = light.wPosition.xyz - wPosition.xyz;
    float lightDistance = length(lightVector);
    if (lightDistance < light.radius) {
        float i = 1 - (lightDistance * lightDistance) / (light.radius * light.radius);

        vec3 l = normalize(lightVector);
        float cosTheta = dot(normal, l);
        if (cosTheta > 0) {
            diffuse += i * id * cosTheta;

            vec3 r = normalize(2 * cosTheta * normal - l);
            float cosOmega = dot(r, v);
            if (cosOmega > 0) {
                specular += i * is * pow(cosOmega, alpha);
            }
        }
    }

    // Combining all lights k
    outColor += (ka * ambient + kd * occlusion * diffuse + ks * specular) * albedo;
}

//----- Implementations

vec3 wPositionFromDepth(float depth, vec2 coord) {
    vec4 pPosition = vec4(2 * coord - 1, depth, 1);
    vec4 vPosition = camera.invertedProjection * pPosition;
    vPosition /= vPosition.w;
    return (camera.invertedView * vPosition).xyz;
}
