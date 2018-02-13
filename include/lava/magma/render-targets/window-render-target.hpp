#pragma once

#include <lava/magma/render-targets/i-render-target.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/core/extent.hpp>
#include <lava/core/ws-event.hpp>
#include <lava/core/ws-handle.hpp>

#include <optional>
#include <string>

namespace lava::magma {
    class RenderEngine;
}

namespace lava::magma {
    /**
     * A window that can be rendered to.
     */
    class WindowRenderTarget final : public IRenderTarget {
    public:
        WindowRenderTarget(RenderEngine& engine, WsHandle handle, const Extent2d& extent);
        ~WindowRenderTarget();

        // IRenderTarget
        IRenderTarget::Impl& interfaceImpl() override final;

        /// Update the render target to the specified dimensions.
        Extent2d extent() const;
        void extent(const Extent2d& extent);

        // Getters
        WsHandle handle() const;

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
