#include "./point-light-impl.hpp"

#include <lava/magma/ubos.hpp>

#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"

using namespace lava::magma;

PointLight::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_descriptorSets(RenderScene::FRAME_IDS_COUNT, nullptr)
    , m_uboHolders(RenderScene::FRAME_IDS_COUNT, m_scene.engine())
{
}

PointLight::Impl::~Impl()
{
    if (m_initialized) {
        for (auto& descriptorSet : m_descriptorSets) {
            m_scene.lightsDescriptorHolder().freeSet(descriptorSet);
        }
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

    for (auto i = 0u; i < m_descriptorSets.size(); ++i) {
        m_descriptorSets[i] =
            m_scene.lightsDescriptorHolder().allocateSet("point-light." + std::to_string(id) + "." + std::to_string(i));
        m_uboHolders[i].init(m_descriptorSets[i], m_scene.lightsDescriptorHolder().uniformBufferBindingOffset(),
                             {sizeof(LightUbo)});
    }

    m_initialized = true;
    m_uboDirty = true;
}

void PointLight::Impl::update()
{
    if (!m_uboDirty) return;

    // :InternalFrameId
    m_currentFrameId = (m_currentFrameId + 1u) % RenderScene::FRAME_IDS_COUNT;

    updateBindings();
}

void PointLight::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                              uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1,
                                     &m_descriptorSets[m_currentFrameId], 0, nullptr);
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
    m_uboDirty = false;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    LightUbo ubo(type());
    ubo.transform = glm::translate(glm::mat4(1.f), m_translation); // @fixme Could be useless, just store that in data
    ubo.data[0].x = reinterpret_cast<const uint32_t&>(m_radius);
    m_uboHolders[m_currentFrameId].copy(0, ubo);
}
