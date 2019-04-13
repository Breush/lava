#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace lava::magma {
    class RenderScene;
    class Texture;
}

namespace lava::magma {
    /**
     * Generic material abstracting internals for uniform bindings.
     */
    class Material final {
    public:
        Material(RenderScene& scene, const std::string& hrid);
        ~Material();

        /// Uniform setters
        void set(const std::string& uniformName, float value);
        void set(const std::string& uniformName, const glm::vec4& value);
        void set(const std::string& uniformName, const Texture& texture);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        RenderScene& m_scene;
        Impl* m_impl = nullptr;
    };
}
