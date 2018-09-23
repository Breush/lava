#include "./directional-light-impl.hpp"

#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../ubos.hpp"

using namespace lava::magma;

DirectionalLight::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
{
}

DirectionalLight::Impl::~Impl()
{
    if (m_initialized) {
        m_scene.lightsDescriptorHolder().freeSet(m_descriptorSet);
    }
}

//----- ILight

RenderImage DirectionalLight::Impl::shadowsRenderImage() const
{
    return m_scene.lightShadowsRenderImage(m_id);
}

//----- ILight::Impl

void DirectionalLight::Impl::init(uint id)
{
    m_id = id;
    m_descriptorSet = m_scene.lightsDescriptorHolder().allocateSet("directional-light." + std::to_string(id));
    m_uboHolder.init(m_descriptorSet, m_scene.lightsDescriptorHolder().uniformBufferBindingOffset(), {sizeof(vulkan::LightUbo)});

    m_initialized = true;
    updateBindings();
}

void DirectionalLight::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                                    uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
}

//----- DirectionalLight

void DirectionalLight::Impl::translation(const glm::vec3& translation)
{
    m_translation = translation;
    updateTransform();
}

void DirectionalLight::Impl::direction(const glm::vec3& direction)
{
    m_direction = glm::normalize(direction);
    updateTransform();
}

//----- Internal

void DirectionalLight::Impl::updateBindings()
{
    if (!m_initialized) return;

    vulkan::LightUbo ubo(type());
    ubo.transform = m_transform;
    ubo.data[0].x = reinterpret_cast<const uint32_t&>(m_direction.x);
    ubo.data[0].y = reinterpret_cast<const uint32_t&>(m_direction.y);
    ubo.data[0].z = reinterpret_cast<const uint32_t&>(m_direction.z);
    m_uboHolder.copy(0, ubo);

    // Bind the shadow map
    const auto binding = m_scene.lightsDescriptorHolder().combinedImageSamplerBindingOffset();
    auto shadowsRenderImage = m_scene.lightShadowsRenderImage(m_id);
    auto imageView = shadowsRenderImage.impl().view();
    auto imageLayout = shadowsRenderImage.impl().layout();

    if (imageView) {
        const auto& sampler = m_scene.engine().shadowsSampler();
        vulkan::updateDescriptorSet(m_scene.engine().device(), m_descriptorSet, imageView, sampler, imageLayout, binding, 0u);
    }
}

void DirectionalLight::Impl::updateTransform()
{
    // @fixme We might need to position the light to scene bounding extremities

    // @note This `projectionTransform[1][1] *= -1` is due to vulkan conventions
    // being different from openGL ones. The Y axis is in the other way.

    auto projectionTransform = glm::ortho(-5.f, 5.f, -5.f, 5.f, 0.01f, 10.f);
    projectionTransform[1][1] *= -1;

    auto viewTransform = glm::lookAt(m_translation, m_translation + m_direction, glm::vec3(0.f, 0.f, 1.f));
    m_transform = projectionTransform * viewTransform;

    updateBindings();
}
