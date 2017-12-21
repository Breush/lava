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

namespace lava::sill {
    class Material {
    public:
        Material(GameEngine& engine, const std::string& hrid);
        ~Material();

        magma::Material& original() { return *m_original; }
        const magma::Material& original() const { return *m_original; }

        /// Uniform setters
        void set(const std::string& uniformName, float value);
        void set(const std::string& uniformName, const glm::vec4& value);
        void set(const std::string& uniformName, const Texture& texture);

    private:
        GameEngine& m_engine;
        magma::Material* m_original = nullptr;
    };
}
