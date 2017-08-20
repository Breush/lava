#pragma once

#include <lava/magma/lights/i-light.hpp>

#include <glm/vec3.hpp>

namespace lava::magma {
    class RenderScene;
}

namespace lava::magma {
    /**
     * An omnidirectional point light.
     */
    class PointLight final : public ILight {
    public:
        PointLight(RenderScene& scene);
        ~PointLight();

        // ILight
        ILight::Impl& interfaceImpl() override final;

        const glm::vec3& position() const;
        void position(const glm::vec3& position);

        float radius() const;
        void radius(float radius);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
