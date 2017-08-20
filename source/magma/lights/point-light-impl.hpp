#pragma once

#include <lava/magma/lights/point-light.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

namespace lava::magma {
    /**
     * Implementation of magma::PointLight.
     */
    class PointLight::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl() = default;

        // IPointLight
        void init();

    private:
        // References
        RenderScene::Impl& m_scene;
        bool m_initialized = false;

        // IPointLight
        $property(glm::vec3, position);
        $property(float, radius, = 1.f);
    };
}
