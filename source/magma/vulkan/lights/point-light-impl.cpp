#include "./point-light-impl.hpp"

#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../ubos.hpp"

using namespace lava::magma;

PointLight::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
{
}

PointLight::Impl::~Impl()
{
    if (m_initialized) {
        m_scene.lightsDescriptorHolder().freeSet(m_descriptorSet);
    }
}

//----- ILight

RenderImage PointLight::Impl::shadowsRenderImage() const
{
    return m_scene.shadowsCascadeRenderImage(m_id);
}

//----- ILight::Impl

void PointLight::Impl::init(uint32_t id)
{
    m_id = id;
    m_descriptorSet = m_scene.lightsDescriptorHolder().allocateSet("point-light." + std::to_string(id));
    m_uboHolder.init(m_descriptorSet, m_scene.lightsDescriptorHolder().uniformBufferBindingOffset(), {sizeof(vulkan::LightUbo)});

    m_initialized = true;
    updateBindings();
}

void PointLight::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                              uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
}

// ----- Attributes

void PointLight::Impl::translation(const glm::vec3& translation)
{
    m_translation = translation;
    updateBindings();
}

void PointLight::Impl::radius(const float& radius)
{
    m_radius = radius;
    updateBindings();
}

// ----- Internal

void PointLight::Impl::updateBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    vulkan::LightUbo ubo(type());
    ubo.transform = glm::translate(glm::mat4(1.f), m_translation); // @fixme Could be useless, just store that in data
    ubo.data[0].x = reinterpret_cast<const uint32_t&>(m_radius);
    m_uboHolder.copy(0, ubo);

    // Bind the shadow map
    // @note Point light currently has no shadows.
    const auto binding = m_scene.lightsDescriptorHolder().combinedImageSamplerBindingOffset();
    const auto& sampler = m_scene.engine().shadowsSampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    const auto imageView = m_scene.engine().dummyImageView();

    vulkan::updateDescriptorSet(m_scene.engine().device(), m_descriptorSet, imageView, sampler, imageLayout, binding, 0u);
}
