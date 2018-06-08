#pragma once

namespace lava::magma {
    /**
     * Holds references to a renderer image.
     *
     * These can be shown within a RenderTarget
     * by adding a view to it through the RenderScene.
     */
    class RenderImage {
    public:
        RenderImage();
        RenderImage(const RenderImage& renderImage);
        ~RenderImage();

        RenderImage& operator=(const RenderImage& renderImage);

        // @note There is nothing usable for the end user, here.
        // All information are stored internally in the impl.

    public:
        class Impl;
        Impl& impl() { return *m_impl; }
        const Impl& impl() const { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
