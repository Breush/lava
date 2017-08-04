#pragma once

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    class IStage {
    public:
        IStage() = default;
        ~IStage() = default;

        /// Called once in a runtime.
        virtual void init() = 0;

        /// Called each time the extent changes.
        virtual void update(const vk::Extent2D& extent) = 0;

        /// Called each frame.
        virtual void render(const vk::CommandBuffer& commandBuffer) = 0;
    };
}
