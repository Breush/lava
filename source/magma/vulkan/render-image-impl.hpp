#pragma once

#include <lava/magma/render-image.hpp>

namespace lava::magma {
    /**
     * Vulkan implementation for RenderImage.
     */
    class RenderImage::Impl {
    public:
        static constexpr auto UUID_CONTEXT_CAMERA = 0x00010000;
        static constexpr auto UUID_CONTEXT_CAMERA_DEPTH = 0x00020000;
        static constexpr auto UUID_CONTEXT_LIGHT_SHADOW_MAP = 0x00030000;

    public:
        /**
         * Used to identify that two render images are supposed to be the same.
         * But one might be the "new one", holding a different view.
         *
         * uuid = UUID_CONTEXT_XXX + localId
         */
        uint32_t uuid() const { return m_uuid; }
        void uuid(uint32_t uuid) { m_uuid = uuid; }

        vk::ImageView view() const { return m_view; }
        void view(vk::ImageView view) { m_view = view; }

        vk::ImageLayout layout() const { return m_layout; }
        void layout(vk::ImageLayout layout) { m_layout = layout; }

    private:
        uint32_t m_uuid = 0u;
        vk::ImageView m_view = nullptr;
        vk::ImageLayout m_layout = vk::ImageLayout::eColorAttachmentOptimal;
    };
}
