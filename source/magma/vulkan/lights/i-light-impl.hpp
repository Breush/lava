#pragma once

#include <lava/magma/lights/i-light.hpp>

#include "../ubos.hpp"

namespace lava::magma {
    enum class LightType {
        Point = 0u,
        Directional = 1u,
    };

    class ILight::Impl {
    public:
        virtual ~Impl() = default;

        virtual void init() = 0;
        virtual LightType type() const = 0;
    };
}
