#pragma once

#include <lava/magma/lights/point-light.hpp>

#include "./i-light-impl.hpp"

#include <lava/core/macros.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

namespace lava::magma {
    /**
     * Implementation of magma::PointLight.
     */
    class PointLight::Impl : public ILight::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl() = default;

        // ILight::Impl
        void init() override final;
        LightType type() const override final { return LightType::Point; };

    private:
        // References
        RenderScene::Impl& m_scene;

        // ILight
        $property(glm::vec3, translation);
        $property(float, radius, = 1.f);
    };
}
