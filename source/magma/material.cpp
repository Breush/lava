#include <lava/magma/material.hpp>

#include <lava/magma/scene.hpp>

#include "./aft-vulkan/material-aft.hpp"

using namespace lava::chamber;
using namespace lava::magma;

Material::Material(Scene& scene, const std::string& hrid)
    : m_scene(scene)
{
    new (&aft()) MaterialAft(*this, m_scene);

    initFromMaterialInfo(hrid);
}

Material::~Material()
{
    aft().~MaterialAft();
}

// ----- Uniform setters

void Material::set(const std::string& uniformName, bool value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.uintValue = (value) ? 1 : 0;
    auto offset = attribute.offset;
    m_ubo.data[offset][0] = (value) ? 1 : 0;
    aft().foreUboChanged();
}

void Material::set(const std::string& uniformName, uint32_t value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.uintValue = value;
    auto offset = attribute.offset;
    m_ubo.data[offset][0] = value;
    aft().foreUboChanged();
}

void Material::set(const std::string& uniformName, float value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.floatValue = value;
    auto offset = attribute.offset;
    reinterpret_cast<float&>(m_ubo.data[offset]) = value;
    aft().foreUboChanged();
}

void Material::set(const std::string& uniformName, const glm::vec2& value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.vec2Value = value;
    auto offset = attribute.offset;
    reinterpret_cast<glm::vec2&>(m_ubo.data[offset]) = value;
    aft().foreUboChanged();
}

void Material::set(const std::string& uniformName, const glm::vec3& value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.vec3Value = value;
    auto offset = attribute.offset;
    reinterpret_cast<glm::vec3&>(m_ubo.data[offset]) = value;
    aft().foreUboChanged();
}

void Material::set(const std::string& uniformName, const glm::vec4& value)
{
    auto& attribute = findAttribute(uniformName);
    attribute.value.vec4Value = value;
    auto offset = attribute.offset;
    reinterpret_cast<glm::vec4&>(m_ubo.data[offset]) = value;
    aft().foreUboChanged();
}

void Material::set(const std::string& uniformName, const Texture& texture)
{
    auto& attribute = findAttribute(uniformName);
    attribute.texture = &texture;
    aft().foreUboChanged();
}

void Material::set(const std::string& uniformName, const uint32_t* values, uint32_t size)
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
    aft().foreUboChanged();
}

// ----- Uniform getters

const glm::vec4& Material::get_vec4(const std::string& uniformName) const
{
    auto& attribute = findAttribute(uniformName);
    return attribute.value.vec4Value;
}

// ----- Inits

void Material::initFromMaterialInfo(const std::string& hrid)
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
        case UniformType::CubeTexture: {
            attribute.offset = 0u;
            break;
        }
        case UniformType::Bool: {
            attribute.offset = basicUniformCount++;
            set(uniformDefinition.name, uniformDefinition.fallback.uintValue);
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

// ----- Finders

Material::Attribute& Material::findAttribute(const std::string& uniformName)
{
    auto pAttribute = m_attributes.find(uniformName);
    if (pAttribute == m_attributes.end()) {
        logger.error("magma.material") << "Attribute '" << uniformName << "' has not been defined." << std::endl;
    }
    return pAttribute->second;
}

const Material::Attribute& Material::findAttribute(const std::string& uniformName) const
{
    auto pAttribute = m_attributes.find(uniformName);
    if (pAttribute == m_attributes.end()) {
        logger.error("magma.material") << "Attribute '" << uniformName << "' has not been defined." << std::endl;
    }
    return pAttribute->second;
}
