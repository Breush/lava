#include "./material-impl.hpp"

#include <cstring>
#include <lava/chamber/logger.hpp>
#include <memory>

#include "./helpers/descriptor.hpp"
#include "./render-engine-impl.hpp"
#include "./render-scenes/render-scene-impl.hpp"
#include "./texture-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

Material::Impl::Impl(RenderScene& scene, const std::string& hrid)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
{
    const auto& materialInfo = m_scene.engine().materialInfo(hrid);
    m_ubo.header.id = materialInfo.id;

    auto basicUniformCount = 0u;
    auto textureUniformCount = 0u;
    for (const auto& uniformDefinition : materialInfo.uniformDefinitions) {
        auto& attribute = m_attributes[uniformDefinition.name];
        attribute.type = uniformDefinition.type;
        attribute.fallback = uniformDefinition.fallback;

        switch (attribute.type) {
        case UniformType::TEXTURE: {
            attribute.offset = textureUniformCount++;
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
    auto& attribute = findAttribute(uniformName);
    auto offset = attribute.offset;
    reinterpret_cast<float&>(m_ubo.data[offset]) = value;
    updateBindings();
}

void Material::Impl::set(const std::string& uniformName, const glm::vec4& value)
{
    auto& attribute = findAttribute(uniformName);
    auto offset = attribute.offset;
    reinterpret_cast<glm::vec4&>(m_ubo.data[offset]) = value;
    updateBindings();
}

void Material::Impl::set(const std::string& uniformName, const Texture& texture)
{
    auto& attribute = findAttribute(uniformName);
    attribute.texture = &texture.impl();
    updateBindings();
}

//----- Private

Material::Impl::Attribute& Material::Impl::findAttribute(const std::string& uniformName)
{
    auto pAttribute = m_attributes.find(uniformName);
    if (pAttribute == m_attributes.end()) {
        logger.error("magma.vulkan.material") << "Attribute '" << uniformName << "' has not been defined." << std::endl;
    }
    return pAttribute->second;
}

void Material::Impl::updateBindings()
{
    if (!m_initialized) return;

    // MaterialUbo
    m_uboHolder.copy(0, m_ubo);

    // Samplers
    const auto& engine = m_scene.engine();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    const auto binding = m_scene.materialDescriptorHolder().combinedImageSamplerBindingOffset();

    for (const auto& attributePair : m_attributes) {
        const auto& attribute = attributePair.second;
        if (attribute.type == UniformType::TEXTURE) {
            vk::ImageView imageView = nullptr;
            if (attribute.texture) {
                imageView = attribute.texture->imageView();
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

            vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, imageView, sampler, imageLayout, binding,
                                        attribute.offset);
        }
    }
}
