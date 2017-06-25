#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
} transforms;

// Should probably be push const
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
    vec3 color;
};
PointLight pointLight = PointLight(vec3(5, 5, 0), vec3(1));

const float PI = 3.1415926535897932384626433832795;

vec4 lightContribution();
vec4 pointLightContribution(PointLight pointLight, vec3 normal);
vec4 bdrf(vec3 cdiff, vec3 F0, float alpha, vec3 lightDirection, vec3 viewDirection, vec3 normal);

void main()
{
    // @todo For each light
    vec3 lightDirection = normalize(pointLight.position - fragWorldPosition);
	vec3 viewDirection = normalize(transforms.cameraPosition - fragWorldPosition);
	vec3 normal = normalize(fragNormal);

    // @fixme Have a way to know if a texture is present, and if not, have the default value
    
    // PBR thingy
    vec4 baseColor = vec4(fragColor * texture(baseColorSampler, fragUv).rgb, 1.0);
    vec2 metallicRoughness = (fragColor * texture(metallicRoughnessSampler, fragUv).rgb).rg;
    float metallic = metallicRoughness.r;
    float roughness = metallicRoughness.g;

    vec3 dielectricSpecular = vec3(0.04);
    vec3 black = vec3(0);
    vec3 cdiff = mix(baseColor.rgb * (1 - dielectricSpecular.r), black, metallic);
    vec3 F0 = mix(dielectricSpecular, baseColor.rgb, metallic);
    float alpha = roughness * roughness;

    vec4 reflectedColor = bdrf(cdiff, F0, alpha, lightDirection, viewDirection, normal);

    // Ambient
    vec4 ambientColor = baseColor * 0.5;

    outColor = ambientColor + reflectedColor;
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

vec4 bdrf(vec3 cdiff, vec3 F0, float alpha, vec3 L, vec3 V, vec3 N)
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

	return vec4(diffuseColor + specularColor, 1.0);
}
