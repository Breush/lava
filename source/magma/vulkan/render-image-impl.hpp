#pragma once

#include <lava/magma/render-image.hpp>

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    /**
     * Vulkan implementation for RenderImage.
     */
    class RenderImage::Impl {
    public:
        vk::ImageView view() const { return m_view; }
        void view(vk::ImageView view) { m_view = view; }

        vk::ImageLayout layout() const { return m_layout; }
        void layout(vk::ImageLayout layout) { m_layout = layout; }

    private:
        vk::ImageView m_view = nullptr;
        vk::ImageLayout m_layout = vk::ImageLayout::eColorAttachmentOptimal;
    };
}
