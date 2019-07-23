#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <lava/magma/aft-infos.hpp>
#include <lava/magma/ubos.hpp>
#include <lava/magma/uniform.hpp>
#include <string>
#include <unordered_map>

namespace lava::magma {
    class MaterialAft;
    class Scene;
    class Texture;
}

namespace lava::magma {
    /**
     * Generic material abstracting internals for uniform bindings.
     */
    class Material {
    public:
        struct Attribute {
            UniformType type = UniformType::Unknown;
            UniformFallback fallback; // Value to use if user does not change it.
            UniformFallback value;    // Last known value.
            uint32_t offset = -1u;
            const Texture* texture = nullptr;
        };
        using Attributes = std::unordered_map<std::string, Attribute>;

    public:
        Material(Scene& scene, const std::string& hrid);
        Material(const Material&) = delete;
        ~Material();

        /// Internal implementation
        MaterialAft& aft() { return reinterpret_cast<MaterialAft&>(m_aft); }
        const MaterialAft& aft() const { return reinterpret_cast<const MaterialAft&>(m_aft); }

        /**
         * @name Uniform setters
         */
        /// @{
        void set(const std::string& uniformName, bool value);
        void set(const std::string& uniformName, uint32_t value);
        void set(const std::string& uniformName, float value);
        void set(const std::string& uniformName, const glm::vec2& value);
        void set(const std::string& uniformName, const glm::vec3& value);
        void set(const std::string& uniformName, const glm::vec4& value);
        void set(const std::string& uniformName, const Texture& texture);
        void set(const std::string& uniformName, const uint32_t* values, uint32_t size);
        /// @}

        /**
         * @name Uniform getters
         */
        /// @{
        const MaterialUbo& ubo() const { return m_ubo; }
        const Attributes& attributes() const { return m_attributes; }

        const glm::vec4& get_vec4(const std::string& uniformName) const;
        /// @}

    private:
        void initFromMaterialInfo(const std::string& hrid);

        Attribute& findAttribute(const std::string& uniformName);
        const Attribute& findAttribute(const std::string& uniformName) const;

    private:
        uint8_t m_aft[MAGMA_SIZEOF_MaterialAft];

        // ----- References
        Scene& m_scene;

        // ----- Shader data
        MaterialUbo m_ubo;

        // ----- Attributes
        Attributes m_attributes;
    };
}
