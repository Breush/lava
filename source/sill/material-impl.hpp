#pragma once

#include <lava/sill/material.hpp>

#include <lava/magma/material.hpp>
#include <lava/sill/game-engine.hpp>

namespace lava::sill {
    class Material::Impl {
    public:
        Impl(GameEngine& engine, const std::string& hrid);
        ~Impl();

        /// Material
        void set(const std::string& uniformName, float value);
        void set(const std::string& uniformName, const glm::vec4& value);
        void set(const std::string& uniformName, const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                 uint8_t channels);

        // Getters
        magma::Material& material() { return *m_material; }
        const magma::Material& material() const { return *m_material; }

    private:
        // References
        GameEngine::Impl& m_engine;

        // Resources
        magma::Material* m_material = nullptr;
    };
}
