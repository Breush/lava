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

//----- Set 1 - Material

layout(set = 1, binding = 0) uniform MaterialUbo {
    float roughnessFactor;
    float metallicFactor;
} material;

#if defined(MAGMA_HAS_NORMAL_SAMPLER)
layout(set = 1, binding = 1) uniform sampler2D tNormalSampler;
#endif
#if defined(MAGMA_HAS_ALBEDO_SAMPLER)
layout(set = 1, binding = 2) uniform sampler2D albedoSampler;
#endif
#if defined(MAGMA_HAS_ORM_SAMPLER)
layout(set = 1, binding = 3) uniform sampler2D ormSampler;
#endif

//----- Fragment forwarded in

layout(location = 0) in mat3 inTbn;
layout(location = 3) in vec2 inUv;

//----- Out data

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outAlbedo;
layout(location = 2) out vec3 outOrm;

//----- Program

void main()
{
    // Normal
    vec3 normal = vec3(0, 0, 1);
#if defined(MAGMA_HAS_NORMAL_SAMPLER)
	normal = 2 * texture(tNormalSampler, inUv).rgb - 1;
#endif

    // Albedo
    vec3 albedo = vec3(1);
#if defined(MAGMA_HAS_ALBEDO_SAMPLER)
    albedo = texture(albedoSampler, inUv).rgb;
#endif

    // Roughness/Metallic
#if defined(MAGMA_HAS_ORM_SAMPLER)
    vec4 orm = texture(ormSampler, inUv);
#endif

    float roughness = material.roughnessFactor;
#if defined(MAGMA_USE_ORM_ROUGHNESS)
    roughness *= orm.g;
#endif

    float metallic = material.metallicFactor;
#if defined(MAGMA_USE_ORM_METALLIC)
    metallic *= orm.b;
#endif

    // Occlusion
    float occlusion = 1;
#if defined(MAGMA_USE_ORM_OCCLUSION)
    occlusion *= orm.r;
#endif

    // Out
    outNormal = ((inTbn * normal) + 1) / 2;
    outAlbedo = albedo;
    outOrm = vec3(occlusion, roughness, metallic);
}
