#version 450
#extension GL_ARB_separate_shader_objects : enable

// @todo These should be generated
#define MAGMA_HAS_NORMAL_SAMPLER
#define MAGMA_HAS_ALBEDO_SAMPLER
#define MAGMA_HAS_ORM_SAMPLER

#if defined(MAGMA_HAS_ORM_SAMPLER)
    #define MAGMA_USE_ORM_OCCLUSION
    #define MAGMA_USE_ORM_ROUGHNESS
    #define MAGMA_USE_ORM_METALLIC
#endif

layout(binding = 0) uniform TransformsUbo {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
} transforms;

layout(binding = 1) uniform AttributesUbo {
    // @todo Colors
    bool dummy;
} attributes;

#if defined(MAGMA_HAS_NORMAL_SAMPLER)
layout(binding = 2) uniform sampler2D normalSampler;
#endif
#if defined(MAGMA_HAS_ALBEDO_SAMPLER)
layout(binding = 3) uniform sampler2D albedoSampler;
#endif
#if defined(MAGMA_HAS_ORM_SAMPLER)
layout(binding = 4) uniform sampler2D ormSampler;
#endif

layout(location = 0) in vec3 fragWorldPosition;
layout(location = 1) in mat3 fragTbn;
layout(location = 4) in vec3 fragColor; // Who cares?
layout(location = 5) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

// @todo Pass this through
struct PointLight {
    vec3 position;
    vec3 color;
};
PointLight pointLight = PointLight(vec3(5, 5, 0), vec3(1));

const float PI = 3.1415926535897932384626433832795;

vec3 bdrf(vec3 cdiff, vec3 F0, float alpha, vec3 lightDirection, vec3 viewDirection, vec3 normal);

void main()
{
    // Samplers
#if defined(MAGMA_HAS_ALBEDO_SAMPLER)
    vec4 albedo = texture(albedoSampler, fragUv);
#endif
#if defined(MAGMA_HAS_ORM_SAMPLER)
    vec4 orm = texture(ormSampler, fragUv);
#endif

    // @todo For each light
    vec3 lightDirection = normalize(pointLight.position - fragWorldPosition);
	vec3 viewDirection = normalize(transforms.cameraPosition - fragWorldPosition);
	vec3 normal = normalize(fragTbn * (texture(normalSampler, fragUv).rgb * 2 - 1));

    // @todo Inverse the TBN matrix in the vertex shader

    // PBR
    vec4 baseColor = vec4(1);
#if defined(MAGMA_HAS_ALBEDO_SAMPLER)
    baseColor *= vec4(fragColor * albedo.rgb, 1.0);
#endif

    float roughness = 1;
    float metallic = 1;

#if defined(MAGMA_USE_ORM_ROUGHNESS)
    roughness *= orm.g;
#endif
#if defined(MAGMA_USE_ORM_METALLIC)
    metallic *= orm.b;
#endif

    vec3 dielectricSpecular = vec3(0.04);
    vec3 cdiff = mix(baseColor.rgb * (1 - dielectricSpecular.r), vec3(0), metallic);
    vec3 F0 = mix(dielectricSpecular, baseColor.rgb, metallic);
    float alpha = roughness * roughness;

    vec3 reflectedColor = bdrf(cdiff, F0, alpha, lightDirection, viewDirection, normal);

    // Ambient
    vec3 ambientColor = baseColor.rgb * 0.5;

    // Occlusion
    float occlusion = 1;
#if defined(MAGMA_USE_ORM_OCCLUSION)
    occlusion *= orm.r;
#endif

    outColor = vec4((ambientColor + reflectedColor) * occlusion, albedo.a);
}

// D = Normal distribution (Distribution of the microfacets)
float D_GGX(float dotNH, float alpha)
{
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return alpha2 / (PI * denom * denom); 
}

// G = Geometric shadowing term (Microfacets shadowing)
float G_SchlicksmithGGX(float dotNL, float dotNV, float alpha)
{
	float r = sqrt(alpha) + 1.0;
	float k = r * r / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// F = Fresnel factor (Reflectance depending on angle of incidence)
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); 
	return F;    
}

vec3 bdrf(vec3 cdiff, vec3 F0, float alpha, vec3 L, vec3 V, vec3 N)
{
    // Diffuse
    vec3 diffuseColor = cdiff / PI;

    // Specular
	// Precalculate vectors and dot products	
	vec3 H = normalize(V + L);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);

	vec3 lightColor = pointLight.color;
	vec3 specularColor = vec3(0.0);

	if (dotNL > 0.0) {
		float D = D_GGX(dotNH, alpha); 
		float G = G_SchlicksmithGGX(dotNL, dotNV, alpha);
		vec3 F = F_Schlick(dotNH, F0);

		vec3 spec = D * F * G / (4.0 * dotNL * dotNV);

		specularColor += spec * dotNL * lightColor;
	}

	return diffuseColor + specularColor;
}
