#pragma once

#include <lava/magma/lights/point-light.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/magma/render-engine.hpp>

namespace lava::magma {
    /**
     * Implementation of magma::PointLight.
     */
    class PointLight::Impl {
    public:
        Impl(RenderEngine& engine);
        ~Impl() = default;

    private:
        // References
        RenderEngine::Impl& m_engine;

        // IPointLight
        $property(glm::vec3, position);
    };
}
