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
    return m_scene.shadowsCascadeRenderImage(m_id);
}

//----- ILight::Impl

void DirectionalLight::Impl::init(uint32_t id)
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

void DirectionalLight::Impl::direction(const glm::vec3& direction)
{
    m_direction = glm::normalize(direction);

    updateBindings();
}

//----- Internal

void DirectionalLight::Impl::updateBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    vulkan::LightUbo ubo(type());
    ubo.data[0].x = reinterpret_cast<const uint32_t&>(m_direction.x);
    ubo.data[0].y = reinterpret_cast<const uint32_t&>(m_direction.y);
    ubo.data[0].z = reinterpret_cast<const uint32_t&>(m_direction.z);
    m_uboHolder.copy(0, ubo);
}
