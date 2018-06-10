#pragma once

#include <lava/magma/render-scenes/i-render-scene.hpp>

#include <lava/magma/render-image.hpp>

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    class IRenderScene::Impl {
    public:
        virtual ~Impl() = default;

        /// Initialize the scene.
        virtual void init(uint32_t id) = 0;

        /// Render the scene.
        virtual void render(vk::CommandBuffer commandBuffer) = 0;
    };
}
