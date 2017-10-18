#include "./material-impl.hpp"

#include <cstring>
#include <lava/chamber/logger.hpp>
#include <memory>

#include "./helpers/descriptor.hpp"
#include "./render-engine-impl.hpp"
#include "./render-scenes/render-scene-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

Material::Impl::Impl(RenderScene& scene, const std::string& hrid)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
{
    const auto& materialInfo = m_scene.engine().materialInfo(hrid);
    m_ubo.header.id = materialInfo.id;

    auto basicUniformCount = 0u;
    for (const auto& uniformDefinition : materialInfo.uniformDefinitions) {
        auto& attribute = m_attributes[uniformDefinition.name];
        attribute.type = uniformDefinition.type;
        attribute.fallback = uniformDefinition.fallback;

        switch (attribute.type) {
        case UniformType::TEXTURE: {
            attribute.offset = m_imageHolders.size();
            auto imageHolder = std::make_unique<vulkan::ImageHolder>(m_scene.engine());
            m_imageHolders.emplace_back(std::move(imageHolder));
            break;
        }
        case UniformType::FLOAT: {
            attribute.offset = basicUniformCount++;
            set(uniformDefinition.name, uniformDefinition.fallback.floatValue);
            break;
        }
        case UniformType::VEC4: {
            attribute.offset = basicUniformCount++;
            set(uniformDefinition.name, uniformDefinition.fallback.vec4Value);
            break;
        }
        default: {
            logger.error("magma.material") << "Uniform definition " << uniformDefinition.name << " has no type specified."
                                           << std::endl;
        }
        }
    }
}

//----- Internals

void Material::Impl::init()
{
    m_descriptorSet = m_scene.materialDescriptorHolder().allocateSet(true);

    m_uboHolder.init(m_descriptorSet, m_scene.materialDescriptorHolder().uniformBufferBindingOffset(),
                     {sizeof(vulkan::MaterialUbo)});

    m_initialized = true;
    updateBindings();
}

void Material::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex)
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
}

//----- Material

void Material::Impl::set(const std::string& uniformName, float value)
{
    auto offset = m_attributes[uniformName].offset;
    reinterpret_cast<float&>(m_ubo.data[offset]) = value;
    updateBindings();
}

void Material::Impl::set(const std::string& uniformName, const glm::vec4& value)
{
    auto offset = m_attributes[uniformName].offset;
    reinterpret_cast<glm::vec4&>(m_ubo.data[offset]) = value;
    updateBindings();
}

// @todo This should be a reference to a texture (holding an ImageHolder), so that it can be shared between materials
void Material::Impl::set(const std::string& uniformName, const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                         uint8_t channels)
{
    auto& attribute = m_attributes[uniformName];
    attribute.enabled = true;
    m_imageHolders[attribute.offset]->setup(pixels, width, height, channels);
    updateBindings();
}

//----- Private

void Material::Impl::updateBindings()
{
    if (!m_initialized) return;

    // MaterialUbo
    m_uboHolder.copy(0, m_ubo);

    // Samplers
    const auto& engine = m_scene.engine();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    const auto offset = m_scene.materialDescriptorHolder().combinedImageSamplerBindingOffset();

    for (const auto& attributePair : m_attributes) {
        const auto& attribute = attributePair.second;
        if (attribute.type == UniformType::TEXTURE) {
            vk::ImageView imageView = nullptr;
            if (attribute.enabled) {
                imageView = m_imageHolders[attribute.offset]->view();
            }
            else if (attribute.fallback.textureTypeValue == UniformTextureType::WHITE) {
                imageView = engine.dummyImageView();
            }
            else if (attribute.fallback.textureTypeValue == UniformTextureType::NORMAL) {
                imageView = engine.dummyNormalImageView();
            }
            else {
                continue;
            }
            vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, imageView, sampler, imageLayout,
                                        offset + attribute.offset);
        }
    }
}
