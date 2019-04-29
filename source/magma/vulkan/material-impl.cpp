#include "./material-impl.hpp"

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
        attribute.value = uniformDefinition.fallback;

        switch (attribute.type) {
        case UniformType::Texture: {
            attribute.offset = textureUniformCount++;
            break;
        }
        case UniformType::Uint: {
            if (uniformDefinition.arraySize == 0u) {
                attribute.offset = basicUniformCount++;
                set(uniformDefinition.name, uniformDefinition.fallback.uintValue);
            }
            else {
                attribute.offset = basicUniformCount;
                basicUniformCount += std::ceil(uniformDefinition.arraySize / 4.f);
                set(uniformDefinition.name, uniformDefinition.fallback.uintArrayValue, uniformDefinition.arraySize);
            }
            break;
        }
        case UniformType::Float: {
            attribute.offset = basicUniformCount++;
            set(uniformDefinition.name, uniformDefinition.fallback.floatValue);
            break;
        }
        case UniformType::Vec2: {
            attribute.offset = basicUniformCount++;
            set(uniformDefinition.name, uniformDefinition.fallback.vec2Value);
            break;
        }
        case UniformType::Vec3: {
            attribute.offset = basicUniformCount++;
            set(uniformDefinition.name, uniformDefinition.fallback.vec3Value);
            break;
        }
        case UniformType::Vec4: {
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

Material::Impl::~Impl()
{
    if (m_initialized) {
        m_scene.materialDescriptorHolder().freeSet(m_descriptorSet);
    }
}

//----- Internals

void Material::Impl::init()
{
    m_descriptorSet = m_scene.materialDescriptorHolder().allocateSet("material", true);

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

void Material::Impl::set(const std::string& uniformName, uint32_t value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.uintValue = value;
    auto offset = attribute.offset;
    m_ubo.data[offset][0] = value;
    updateBindings();
}

void Material::Impl::set(const std::string& uniformName, float value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.floatValue = value;
    auto offset = attribute.offset;
    reinterpret_cast<float&>(m_ubo.data[offset]) = value;
    updateBindings();
}

void Material::Impl::set(const std::string& uniformName, const glm::vec2& value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.vec2Value = value;
    auto offset = attribute.offset;
    reinterpret_cast<glm::vec2&>(m_ubo.data[offset]) = value;
    updateBindings();
}

void Material::Impl::set(const std::string& uniformName, const glm::vec3& value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.vec3Value = value;
    auto offset = attribute.offset;
    reinterpret_cast<glm::vec3&>(m_ubo.data[offset]) = value;
    updateBindings();
}

void Material::Impl::set(const std::string& uniformName, const glm::vec4& value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.vec4Value = value;
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

void Material::Impl::set(const std::string& uniformName, const uint32_t* values, uint32_t size)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value = UniformFallback(values, size);
    auto offset = attribute.offset;
    for (auto i = 0u; i < size; i += 4u) {
        auto& data = m_ubo.data[offset + i / 4u];
        data[0] = values[i];
        if (i + 1 < size) data[1] = values[i + 1];
        if (i + 2 < size) data[2] = values[i + 2];
        if (i + 3 < size) data[3] = values[i + 3];
    }
    updateBindings();
}

const glm::vec4& Material::Impl::get_vec4(const std::string& uniformName) const
{
    auto& attribute = findAttribute(uniformName);
    return attribute.value.vec4Value;
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

const Material::Impl::Attribute& Material::Impl::findAttribute(const std::string& uniformName) const
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
    auto imageView = engine.dummyImageView();

    // Force all samplers to white image view by default.
    for (auto i = 0u; i < MATERIAL_SAMPLERS_SIZE; ++i) {
        vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, imageView, sampler, imageLayout, binding, i);
    }

    for (const auto& attributePair : m_attributes) {
        const auto& attribute = attributePair.second;
        if (attribute.type == UniformType::Texture) {
            if (attribute.texture) {
                imageView = attribute.texture->imageView();
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

            vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, imageView, sampler, imageLayout, binding,
                                        attribute.offset);
        }
    }
}
