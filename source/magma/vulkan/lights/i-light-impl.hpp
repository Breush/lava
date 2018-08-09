#pragma once

#include <lava/magma/lights/i-light.hpp>
#include <vulkan/vulkan.hpp>

#include "../ubos.hpp"

namespace lava::magma {
    enum class LightType {
        Point = 0u,
        Directional = 1u,
    };

    class ILight::Impl {
    public:
        virtual ~Impl() = default;

        virtual void init(uint32_t id) = 0;
        virtual LightType type() const = 0;

        /// Render the light for shadows.
        virtual void renderShadows(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                                   uint32_t descriptorSetIndex) const = 0;
    };
}
