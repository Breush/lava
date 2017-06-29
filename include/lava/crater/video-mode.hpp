#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>

namespace lava {
    ////////////////////////////////////////////////////////////
    /// \brief VideoMode defines a video mode (width, height, bpp)
    ///
    ////////////////////////////////////////////////////////////
    class VideoMode {
    public:
        ////////////////////////////////////////////////////////////
        /// \brief Default constructor
        ///
        /// This constructors initializes all members to 0.
        ///
        ////////////////////////////////////////////////////////////
        VideoMode();

        ////////////////////////////////////////////////////////////
        /// \brief Construct the video mode with its attributes
        ///
        /// \param modeWidth        Width in pixels
        /// \param modeHeight       Height in pixels
        /// \param modeBitsPerPixel Pixel depths in bits per pixel
        ///
        ////////////////////////////////////////////////////////////
        VideoMode(unsigned int modeWidth, unsigned int modeHeight, unsigned int modeBitsPerPixel = 32);

        ////////////////////////////////////////////////////////////
        // Member data
        ////////////////////////////////////////////////////////////
        unsigned int width;        ///< Video mode width, in pixels
        unsigned int height;       ///< Video mode height, in pixels
        unsigned int bitsPerPixel; ///< Video mode pixel depth, in bits per pixels
    };
}
