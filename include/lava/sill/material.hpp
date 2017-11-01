#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace lava::sill {
    class GameEngine;
    class Texture;
}

namespace lava::sill {
    class Material {
    public:
        Material(GameEngine& engine, const std::string& hrid);
        ~Material();

        /// Uniform setters
        void set(const std::string& uniformName, float value);
        void set(const std::string& uniformName, const glm::vec4& value);
        void set(const std::string& uniformName, const Texture& texture);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
