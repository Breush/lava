#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace lava::magma {
    class Material;
}

namespace lava::sill {
    class GameEngine;
    class Texture;
}

// @fixme With Texture, should not expose magma here...

namespace lava::sill {
    class Material {
    public:
        Material(GameEngine& engine, const std::string& hrid);
        ~Material();

        magma::Material& magma() { return *m_magma; }
        const magma::Material& magma() const { return *m_magma; }

        /// Uniform setters
        void set(const std::string& uniformName, float value);
        void set(const std::string& uniformName, const glm::vec4& value);
        void set(const std::string& uniformName, const Texture& texture);

    private:
        GameEngine& m_engine;
        magma::Material* m_magma = nullptr;
    };
}
