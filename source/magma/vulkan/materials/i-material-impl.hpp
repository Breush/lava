#pragma once

#include <lava/magma/materials/i-material.hpp>

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    /**
     * Interface for materials.
     */
    class IMaterial::Impl {
    public:
        virtual ~Impl() = default;

        virtual void init() = 0;

        /// Render the material.
        virtual void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) = 0;
    };
}
