#pragma once

#include <lava/magma/interfaces/render-scene.hpp>

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    // @todo What's the point of IRenderScene at all, then?
    // It just hold just what's necessary to the end user.
    // And that means nothing at all.
    // Or, it should hold what's necessary to a implementation-agnostic
    // RenderEngine. And that might be the main render loop.
    class IRenderScene::Impl {
    public:
        virtual ~Impl() = default;

        // IRenderScene
        virtual void init() = 0;

        virtual Extent2d extent() const = 0;
        virtual void extent(Extent2d extent) = 0;

        // Implementation internals

        /// Render the scene.
        virtual void render(vk::CommandBuffer commandBuffer) = 0;

        /// The final rendered image view.
        virtual vk::ImageView imageView() const = 0;
    };
}
