#pragma once

#include <lava/magma/render-targets/i-render-target.hpp>

#include <lava/core/extent.hpp>
#include <lava/core/macros.hpp>

#include <optional>
#include <string>

namespace lava::magma {
    class RenderEngine;
    class RenderScene;
}

namespace lava::magma {
    /**
     * A render target for VR head-mounted devices.
     */
    class VrRenderTarget final : public IRenderTarget {
    public:
        VrRenderTarget(RenderEngine& engine);
        ~VrRenderTarget();

        // IRenderTarget
        IRenderTarget::Impl& interfaceImpl() override final;

        /**
         * Decides which scene the VR should render.
         *
         * Please note that 3D RenderScene is used here.
         * This is because a 3D context is needed for the VR to render
         * coherently.
         */
        void bindScene(RenderScene& scene);

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
