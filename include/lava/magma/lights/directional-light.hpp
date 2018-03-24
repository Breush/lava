#pragma once

#include <lava/magma/lights/i-light.hpp>

#include <glm/vec3.hpp>

namespace lava::magma {
    class RenderScene;
}

namespace lava::magma {
    /**
     * A directional light.
     */
    class DirectionalLight final : public ILight {
    public:
        DirectionalLight(RenderScene& scene);
        ~DirectionalLight();

        // ILight
        ILight::Impl& interfaceImpl() override final;

        // In order to cast shadows, the light needs a position
        const glm::vec3& position() const;
        void position(const glm::vec3& position);

        const glm::vec3& direction() const;
        void direction(const glm::vec3& direction);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
