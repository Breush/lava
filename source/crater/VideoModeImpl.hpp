#pragma once

#include <lava/crater/VideoMode.hpp>

namespace lava::priv {

    /// \brief OS-specific implementation of video modes functions
    class VideoModeImpl {
    public:
        /// \brief Get the list of all the supported fullscreen video modes
        ///
        /// \return Array filled with the fullscreen video modes
        static std::vector<VideoMode> getFullscreenModes();

        /// \brief Get the current desktop video mode
        ///
        /// \return Current desktop video mode
        static VideoMode getDesktopMode();
    };
}
