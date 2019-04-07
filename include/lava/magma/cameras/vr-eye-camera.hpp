#pragma once

#include <lava/magma/cameras/i-camera.hpp>

#include <glm/glm.hpp>

namespace lava::magma {
    class RenderScene;
}

namespace lava::magma {
    /**
     * Renders an eye for a VR headset.
     */
    class VrEyeCamera final : public ICamera {
    public:
        VrEyeCamera(RenderScene& scene, Extent2d extent);
        ~VrEyeCamera();

        // ICamera
        Extent2d extent() const final;
        void extent(Extent2d extent) final;
        RenderImage renderImage() const final;
        RenderImage depthRenderImage() const final;
        PolygonMode polygonMode() const final;
        void polygonMode(PolygonMode polygonMode) final;

        ICamera::Impl& interfaceImpl();

        // @note There is no control, as this camera is expected to be used
        // only in internal code. However, one can still configure it.

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
