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
        RenderImage shadowsRenderImage() const override final;
        ILight::Impl& interfaceImpl() override final;

        // @note Shadows for point lights are currently not handled.
        bool shadowsEnabled() const override final { return false; }

        const glm::vec3& translation() const;
        void translation(const glm::vec3& translation);

        float radius() const;
        void radius(float radius);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
