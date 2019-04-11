#pragma once

#include <lava/magma/render-scenes/i-render-scene.hpp>

#include <lava/magma/render-image.hpp>

namespace lava::magma {
    // @fixme Why whould that IRenderScene::Impl would be useful?
    class IRenderScene::Impl {
    public:
        virtual ~Impl() = default;

        /// Initialize the scene.
        virtual void init(uint32_t id) = 0;

        /// Render the scene. Fills the commandBuffers array.
        virtual void record() = 0;

        /// All generated command buffers since last call.
        virtual const std::vector<vk::CommandBuffer>& commandBuffers() const = 0;
    };
}
