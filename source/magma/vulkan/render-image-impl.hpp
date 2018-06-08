#pragma once

#include <lava/magma/render-image.hpp>

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    /**
     * Vulkan implementation for RenderImage.
     */
    class RenderImage::Impl {
    public:
        vk::ImageView imageView() const { return m_imageView; }
        void imageView(vk::ImageView imageView) { m_imageView = imageView; }

        vk::ImageLayout imageLayout() const { return m_imageLayout; }
        void imageLayout(vk::ImageLayout imageLayout) { m_imageLayout = imageLayout; }

    private:
        vk::ImageView m_imageView = nullptr;
        vk::ImageLayout m_imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    };
}
