#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <lava/core/macros/aft.hpp>
#include <lava/magma/ubos.hpp>
#include <lava/magma/uniform.hpp>
#include <string>
#include <unordered_map>

namespace lava::magma {
    class MaterialAft;
    class Scene;
    class Texture;

    using TexturePtr = std::shared_ptr<Texture>;
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
            TexturePtr texture = nullptr;
        };
        using Attributes = std::unordered_map<std::string, Attribute>;

    public:
        Material(Scene& scene, const std::string& hrid);
        ~Material();

        $aft_class(Material);

        const std::string& hrid() const { return m_hrid; }

        const std::string& name() const { return m_name; }
        void name(const std::string& name) { m_name = name; }

        Scene& scene() { return m_scene; }
        const Scene& scene() const { return m_scene; }

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
        void set(const std::string& uniformName, const TexturePtr& texture);
        void set(const std::string& uniformName, const uint32_t* values, uint32_t size);

        void setGlobal(const std::string& uniformName, const TexturePtr& texture);
        /// @}

        /**
         * @name Uniform getters
         */
        /// @{
        const MaterialUbo& ubo() const { return m_ubo; }
        const Attributes& globalAttributes() const { return s_globalAttributes.at(m_hrid); }
        const Attributes& attributes() const { return m_attributes; }

        float get_float(const std::string& uniformName) const;
        const glm::vec4& get_vec4(const std::string& uniformName) const;
        /// @}

    private:
        void initFromMaterialInfo(const std::string& hrid);
        void initAttributes(Attributes& attributes, const UniformDefinitions& uniformDefinitions);

        Attribute& findAttribute(const std::string& uniformName);
        const Attribute& findAttribute(const std::string& uniformName) const;

        Attribute& findGlobalAttribute(const std::string& uniformName);
        const Attribute& findGlobalAttribute(const std::string& uniformName) const;

    private:
        // ----- References
        Scene& m_scene;
        std::string m_hrid;
        std::string m_name;

        // ----- Shader data
        MaterialUbo m_ubo;

        // ----- Attributes
        static std::unordered_map<std::string, Attributes> s_globalAttributes;
        Attributes m_attributes;
    };

    using MaterialPtr = std::shared_ptr<Material>;
}
