#pragma once

#include <lava/magma/lights/i-light.hpp>

#include "../../light-type.hpp"

namespace lava::magma {
    class ILight::Impl {
    public:
        virtual ~Impl() = default;

        virtual void init(uint32_t id) = 0;
        virtual void update() = 0;
        virtual LightType type() const = 0;

        /// Bind the light descriptor.
        virtual void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                            uint32_t descriptorSetIndex) const = 0;
    };
}
