#pragma once

#include <lava/magma/lights/directional-light.hpp>

#include "./i-light-impl.hpp"

#include <lava/core/macros.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

namespace lava::magma {
    /**
     * Implementation of magma::DirectionalLight.
     */
    class DirectionalLight::Impl : public ILight::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl() = default;

        // ILight::Impl
        void init() override final;
        LightType type() const override final { return LightType::Directional; };

    private:
        // References
        RenderScene::Impl& m_scene;

        // IDirectionalLight
        $property(glm::vec3, translation);
        $property(glm::vec3, direction);
    };
}
