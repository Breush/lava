#version 450
#extension GL_ARB_separate_shader_objects : enable

// Should probably be push const
layout(binding = 1) uniform sampler2D baseColorSampler;
layout(binding = 2) uniform sampler2D metallicRoughnessSampler;

layout(location = 0) in vec3 fragCameraPosition;
layout(location = 1) in vec3 fragWorldPosition;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragColor;
layout(location = 4) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

// Pass this through
struct PointLight {
    vec3 position;
    vec3 color;
};
PointLight pointLight = PointLight(vec3(-5, 5, -10), vec3(1));

vec4 lightContribution();
vec4 pointLightContribution(PointLight pointLight, vec3 normal);
vec4 BDRF(vec3 cdiff, vec3 F0, float alpha, vec3 lightDirection, vec3 viewDirection, vec3 normal);

void main()
{
    vec4 lightColor = lightContribution();

    // @todo For each light
    vec3 lightDirection = normalize(pointLight.position - fragWorldPosition);
	vec3 viewDirection = normalize(fragCameraPosition - fragWorldPosition);
	vec3 normal = normalize(fragNormal);

    // @fixme Have a way to know if a texture is present, and if not, have the default value
    
    // PBR thingy
    vec4 baseColor = vec4(fragColor * texture(baseColorSampler, fragUv).rgb, 1.0);
    vec2 metallicRoughness = (fragColor * texture(metallicRoughnessSampler, fragUv).rgb).rg;
    float metallic = metallicRoughness.r;
    float roughness = metallicRoughness.g;

    // glTF Specification
    vec3 dielectricSpecular = vec3(0.04);
    vec3 black = vec3(0);
    vec3 cdiff = mix(baseColor.rgb * (1 - dielectricSpecular.r), black, metallic);
    vec3 F0 = mix(dielectricSpecular, baseColor.rgb, metallic);
    float alpha = roughness * roughness;

    // Computation
    vec4 pbrColor = BDRF(cdiff, F0, alpha, lightDirection, viewDirection, normal);

    outColor = lightColor * baseColor * pbrColor;
}

// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float alpha)
{
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float alpha)
{
	float r = (sqrt(alpha) + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); 
	return F;    
}

vec4 BDRF(vec3 cdiff, vec3 F0, float alpha, vec3 lightDirection, vec3 viewDirection, vec3 normal)
{
    // @todo cdiff unused

    vec3 L = lightDirection;
    vec3 V = viewDirection;
    vec3 N = normal;

	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);

	vec3 lightColor = pointLight.color;
	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, alpha); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, alpha);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, F0);

		vec3 spec = D * F * G / (4.0 * dotNL * dotNV);

		color += spec * dotNL * lightColor;
	}

	return color;
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
