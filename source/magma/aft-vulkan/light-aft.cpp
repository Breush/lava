#include "./light-aft.hpp"

#include <lava/magma/light.hpp>
#include <lava/magma/scene.hpp>

#include "./scene-aft.hpp"

using namespace lava::chamber;
using namespace lava::magma;

LightAft::LightAft(Light& fore, Scene& scene)
    : m_fore(fore)
    , m_scene(scene)
    , m_descriptorSets(make_array<FRAME_IDS_COUNT, vk::DescriptorSet>(nullptr))
    , m_uboHolders(make_array<FRAME_IDS_COUNT, vulkan::UboHolder>(m_scene.engine().impl()))
{
}

void LightAft::init()
{
    auto& descriptorHolder = m_scene.aft().lightsDescriptorHolder();
    for (auto i = 0u; i < m_descriptorSets.size(); ++i) {
        m_descriptorSets[i] = descriptorHolder.allocateSet("light." + std::to_string(i));
        m_uboHolders[i].init(m_descriptorSets[i], descriptorHolder.uniformBufferBindingOffset(), {sizeof(LightUbo)});
    }

    m_uboDirty = true;
}

void LightAft::update()
{
    if (!m_uboDirty) return;

    // :InternalFrameId
    m_currentFrameId = (m_currentFrameId + 1u) % FRAME_IDS_COUNT;

    updateBindings();
}

void LightAft::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1,
                                     &m_descriptorSets[m_currentFrameId], 0, nullptr);
}

// ----- Fore

RenderImage LightAft::foreShadowsRenderImage() const
{
    return m_scene.aft().shadowsCascadeRenderImage(m_fore);
}

// ----- Updates

void LightAft::updateBindings()
{
    m_uboDirty = false;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_uboHolders[m_currentFrameId].copy(0, m_fore.ubo());
}
