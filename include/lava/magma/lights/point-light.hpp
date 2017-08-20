#pragma once

#include <lava/magma/interfaces/point-light.hpp>

#include <glm/vec3.hpp>

namespace lava::magma {
    class RenderScene;
}

namespace lava::magma {
    /**
     * An omnidirectional point light.
     */
    class PointLight final : public IPointLight {
    public:
        PointLight(RenderScene& scene);
        ~PointLight();

        // IPointLight
        void init() override final;
        const glm::vec3& position() const override final;
        float radius() const override final;

        void position(const glm::vec3& position);
        void radius(float radius);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
