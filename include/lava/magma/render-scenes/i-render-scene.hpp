#pragma once

#include <lava/magma/extent.hpp>

namespace lava::magma {
    /**
     * Interface for render scenes.
     */
    class IRenderScene {
    public:
        virtual ~IRenderScene() = default;

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
