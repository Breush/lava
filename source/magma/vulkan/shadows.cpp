#include "./shadows.hpp"

#include <lava/magma/camera.hpp>
#include <lava/magma/light.hpp>
#include <lava/magma/scene.hpp>

#include "../aft-vulkan/config.hpp"
#include "../aft-vulkan/scene-aft.hpp"
#include "./render-engine-impl.hpp"
#include "./render-image-impl.hpp"

using namespace lava::magma;

Shadows::Shadows(Scene& scene)
    : m_scene(scene)
    , m_uboHolders(FRAME_IDS_COUNT, scene.engine().impl())
    , m_descriptorSets(FRAME_IDS_COUNT, nullptr)
{
}

Shadows::~Shadows()
{
    if (m_initialized) {
        for (auto& descriptorSet : m_descriptorSets) {
            m_scene.aft().shadowsDescriptorHolder().freeSet(descriptorSet);
        }
    }
}

void Shadows::init(const Light& light, const Camera& camera)
{
    if (m_initialized) return;
    m_initialized = true;

    m_camera = &camera;
    m_light = &light;

    auto& descriptorHolder = m_scene.aft().shadowsDescriptorHolder();
    for (auto i = 0u; i < m_descriptorSets.size(); ++i) {
        m_descriptorSets[i] = descriptorHolder.allocateSet("shadows." + std::to_string(i));
        m_uboHolders[i].init(m_descriptorSets[i], descriptorHolder.uniformBufferBindingOffset(), {sizeof(ShadowsUbo)});
    }

    updateImagesBindings();
}

void Shadows::update(uint32_t frameId)
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    float cascadeSplitLambda = 0.95f;
    float cascadeSplits[SHADOWS_CASCADES_COUNT];

    float nearClip = m_camera->nearClip();
    float farClip = m_camera->farClip();
    const auto& viewProjectionTransformInverse = m_camera->viewProjectionTransformInverse();

    float clipRange = farClip - nearClip;

    float minZ = nearClip;
    float maxZ = nearClip + clipRange;

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;

    // @todo Currently handling only directional lights!
    // What's the good thing to do?
    glm::vec3 lightDir = m_light->shadowsDirection();

    // Calculate split depths based on view camera frustum
    // Based on method presentd in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    for (uint32_t i = 0; i < SHADOWS_CASCADES_COUNT; i++) {
        float p = (i + 1) / static_cast<float>(SHADOWS_CASCADES_COUNT);
        float log = minZ * std::pow(ratio, p);
        float uniform = minZ + range * p;
        float d = cascadeSplitLambda * (log - uniform) + uniform;
        cascadeSplits[i] = (d - nearClip) / clipRange;
    }

    // Calculate orthographic projection matrix for each cascade
    float lastSplitDist = 0.0;
    for (uint32_t i = 0; i < SHADOWS_CASCADES_COUNT; i++) {
        float splitDist = cascadeSplits[i];

        glm::vec3 frustumCorners[8] = {
            glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, -1.0f),  glm::vec3(1.0f, -1.0f, -1.0f),
            glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, 1.0f, 1.0f),  glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, -1.0f, 1.0f),   glm::vec3(-1.0f, -1.0f, 1.0f),
        };

        // Project frustum corners into world space
        for (uint32_t i = 0; i < 8; i++) {
            glm::vec4 invCorner = viewProjectionTransformInverse * glm::vec4(frustumCorners[i], 1.0f);
            frustumCorners[i] = invCorner / invCorner.w;
        }

        for (uint32_t i = 0; i < 4; i++) {
            glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
            frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
        }

        // Get frustum center
        glm::vec3 frustumCenter = glm::vec3(0.0f);
        for (uint32_t i = 0; i < 8; i++) {
            frustumCenter += frustumCorners[i];
        }
        frustumCenter /= 8.0f;

        float radius = 0.0f;
        for (uint32_t i = 0; i < 8; i++) {
            float distance = glm::length(frustumCorners[i] - frustumCenter);
            radius = glm::max(radius, distance);
        }
        radius = std::ceil(radius * 16.0f) / 16.0f;

        glm::vec3 maxExtents = glm::vec3(radius);
        glm::vec3 minExtents = -maxExtents;

        glm::mat4 lightViewMatrix =
            glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightOrthoMatrix =
            glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

        // Store split distance and matrix in cascade
        m_cascades[i].splitDepth = (nearClip + splitDist * clipRange);
        m_cascades[i].transform = lightOrthoMatrix * lightViewMatrix;

        lastSplitDist = cascadeSplits[i];
    }

    updateBindings(frameId);
}

void Shadows::render(vk::CommandBuffer commandBuffer, uint32_t frameId, vk::PipelineLayout pipelineLayout,
                     uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1,
                                     &m_descriptorSets[frameId], 0, nullptr);
}

void Shadows::updateImagesBindings()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // Bind the maps, just once, as these never change
    auto& engine = m_scene.engine().impl();
    const auto binding = m_scene.aft().shadowsDescriptorHolder().combinedImageSamplerBindingOffset();
    const auto& sampler = engine.shadowsSampler();

    for (auto cascadeIndex = 0u; cascadeIndex < SHADOWS_CASCADES_COUNT; ++cascadeIndex) {
        auto shadowsRenderImage = m_scene.aft().shadowsCascadeRenderImage(*m_light, m_camera, cascadeIndex);
        auto imageView = shadowsRenderImage.impl().view();
        auto imageLayout = shadowsRenderImage.impl().layout();

        if (imageView) {
            for (auto& descriptorSet : m_descriptorSets) {
                vulkan::updateDescriptorSet(engine.device(), descriptorSet, imageView, sampler, imageLayout, binding,
                                            cascadeIndex);
            }
        }
    }
}

void Shadows::updateBindings(uint32_t frameId)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // Bind the splits
    ShadowsUbo ubo;
    for (auto i = 0u; i < SHADOWS_CASCADES_COUNT; ++i) {
        ubo.cascadesTransforms[i] = m_scene.aft().shadowsCascadeTransform(*m_light, *m_camera, i);
        ubo.cascadesSplits[i][0] = m_scene.aft().shadowsCascadeSplitDepth(*m_light, *m_camera, i);
    }

    m_uboHolders[frameId].copy(0, ubo);
}
