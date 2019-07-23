#include "./material-aft.hpp"

#include <lava/magma/material.hpp>
#include <lava/magma/scene.hpp>
#include <lava/magma/texture.hpp>

#include "../vulkan/render-engine-impl.hpp"
#include "./scene-aft.hpp"
#include "./texture-aft.hpp"

using namespace lava::chamber;
using namespace lava::magma;

MaterialAft::MaterialAft(Material& fore, Scene& scene)
    : m_fore(fore)
    , m_scene(scene)
    , m_descriptorSets(make_array<FRAME_IDS_COUNT, vk::DescriptorSet>(nullptr))
    , m_uboHolders(make_array<FRAME_IDS_COUNT, vulkan::UboHolder>(m_scene.engine().impl()))
{
}

MaterialAft::~MaterialAft()
{
    for (auto& descriptorSet : m_descriptorSets) {
        m_scene.aft().materialDescriptorHolder().freeSet(descriptorSet);
    }
}

void MaterialAft::init()
{
    auto& descriptorHolder = m_scene.aft().materialDescriptorHolder();
    for (auto i = 0u; i < m_descriptorSets.size(); ++i) {
        m_descriptorSets[i] = descriptorHolder.allocateSet("material." + std::to_string(i), true);
        m_uboHolders[i].init(m_descriptorSets[i], descriptorHolder.uniformBufferBindingOffset(), {sizeof(MaterialUbo)});
    }

    m_uboDirty = true;
}

void MaterialAft::update()
{
    if (!m_uboDirty) return;

    // :InternalFrameId @note The idea is to be sure that the material's UBO is not in use
    // while we update it. We do that by updating it into a different uboHolder/descriptorSet.
    m_currentFrameId = (m_currentFrameId + 1u) % FRAME_IDS_COUNT;

    updateBindings();
}

void MaterialAft::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1,
                                     &m_descriptorSets[m_currentFrameId], 0, nullptr);
}

// ----- Updates

void MaterialAft::updateBindings()
{
    m_uboDirty = false;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto& descriptorSet = m_descriptorSets[m_currentFrameId];

    // MaterialUbo
    m_uboHolders[m_currentFrameId].copy(0, m_fore.ubo());

    // Samplers
    const auto& engine = m_scene.engine().impl();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    auto& descriptorHolder = m_scene.aft().materialDescriptorHolder();
    const auto binding = descriptorHolder.combinedImageSamplerBindingOffset();
    auto imageView = engine.dummyImageView();

    // Force all samplers to white image view by default.
    for (auto i = 0u; i < MATERIAL_SAMPLERS_SIZE; ++i) {
        vulkan::updateDescriptorSet(engine.device(), descriptorSet, imageView, sampler, imageLayout, binding, i);
    }

    for (const auto& attributePair : m_fore.attributes()) {
        const auto& attribute = attributePair.second;
        if (attribute.type == UniformType::Texture) {
            if (attribute.texture) {
                imageView = attribute.texture->aft().imageView();
            }
            else if (attribute.fallback.textureTypeValue == UniformTextureType::Normal) {
                imageView = engine.dummyNormalImageView();
            }
            else if (attribute.fallback.textureTypeValue == UniformTextureType::Invisible) {
                imageView = engine.dummyInvisibleImageView();
            }
            else {
                continue;
            }

            // @fixme We could use that, and updateDescriptorSet should be useless
            // m_scene.materialDescriptorHolder().updateSet(descriptorSet, imageView, imageLayout, )
            vulkan::updateDescriptorSet(engine.device(), descriptorSet, imageView, sampler, imageLayout, binding,
                                        attribute.offset);
        }
    }

    //----- Cube samplers

    // Force white cube
    auto cubeImageView = engine.dummyCubeImageView();
    vulkan::updateDescriptorSet(engine.device(), descriptorSet, cubeImageView, sampler, imageLayout, binding + 1);

    for (const auto& attributePair : m_fore.attributes()) {
        const auto& attribute = attributePair.second;
        if (attribute.type == UniformType::CubeTexture) {
            if (attribute.texture) {
                cubeImageView = attribute.texture->aft().imageView();
            }
            else {
                continue;
            }

            vulkan::updateDescriptorSet(engine.device(), descriptorSet, cubeImageView, sampler, imageLayout, binding + 1);
        }
    }
}
