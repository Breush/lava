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
        /// \brief Get the current desktop video mode
        ///
        /// \return Current desktop video mode
        ///
        ////////////////////////////////////////////////////////////
        static VideoMode getDesktopMode();

        ////////////////////////////////////////////////////////////
        /// \brief Retrieve all the video modes supported in fullscreen mode
        ///
        /// When creating a fullscreen window, the video mode is restricted
        /// to be compatible with what the graphics driver and monitor
        /// support. This function returns the complete list of all video
        /// modes that can be used in fullscreen mode.
        /// The returned array is sorted from best to worst, so that
        /// the first element will always give the best mode (higher
        /// width, height and bits-per-pixel).
        ///
        /// \return Array containing all the supported fullscreen modes
        ///
        ////////////////////////////////////////////////////////////
        static const std::vector<VideoMode>& getFullscreenModes();

        ////////////////////////////////////////////////////////////
        /// \brief Tell whether or not the video mode is valid
        ///
        /// The validity of video modes is only relevant when using
        /// fullscreen windows; otherwise any video mode can be used
        /// with no restriction.
        ///
        /// \return True if the video mode is valid for fullscreen mode
        ///
        ////////////////////////////////////////////////////////////
        bool isValid() const;

        ////////////////////////////////////////////////////////////
        // Member data
        ////////////////////////////////////////////////////////////
        unsigned int width;        ///< Video mode width, in pixels
        unsigned int height;       ///< Video mode height, in pixels
        unsigned int bitsPerPixel; ///< Video mode pixel depth, in bits per pixels
    };

    ////////////////////////////////////////////////////////////
    /// \relates VideoMode
    /// \brief Overload of == operator to compare two video modes
    ///
    /// \param left  Left operand (a video mode)
    /// \param right Right operand (a video mode)
    ///
    /// \return True if modes are equal
    ///
    ////////////////////////////////////////////////////////////
    bool operator==(const VideoMode& left, const VideoMode& right);

    ////////////////////////////////////////////////////////////
    /// \relates VideoMode
    /// \brief Overload of != operator to compare two video modes
    ///
    /// \param left  Left operand (a video mode)
    /// \param right Right operand (a video mode)
    ///
    /// \return True if modes are different
    ///
    ////////////////////////////////////////////////////////////
    bool operator!=(const VideoMode& left, const VideoMode& right);

    ////////////////////////////////////////////////////////////
    /// \relates VideoMode
    /// \brief Overload of < operator to compare video modes
    ///
    /// \param left  Left operand (a video mode)
    /// \param right Right operand (a video mode)
    ///
    /// \return True if \a left is lesser than \a right
    ///
    ////////////////////////////////////////////////////////////
    bool operator<(const VideoMode& left, const VideoMode& right);

    ////////////////////////////////////////////////////////////
    /// \relates VideoMode
    /// \brief Overload of > operator to compare video modes
    ///
    /// \param left  Left operand (a video mode)
    /// \param right Right operand (a video mode)
    ///
    /// \return True if \a left is greater than \a right
    ///
    ////////////////////////////////////////////////////////////
    bool operator>(const VideoMode& left, const VideoMode& right);

    ////////////////////////////////////////////////////////////
    /// \relates VideoMode
    /// \brief Overload of <= operator to compare video modes
    ///
    /// \param left  Left operand (a video mode)
    /// \param right Right operand (a video mode)
    ///
    /// \return True if \a left is lesser or equal than \a right
    ///
    ////////////////////////////////////////////////////////////
    bool operator<=(const VideoMode& left, const VideoMode& right);

    ////////////////////////////////////////////////////////////
    /// \relates VideoMode
    /// \brief Overload of >= operator to compare video modes
    ///
    /// \param left  Left operand (a video mode)
    /// \param right Right operand (a video mode)
    ///
    /// \return True if \a left is greater or equal than \a right
    ///
    ////////////////////////////////////////////////////////////
    bool operator>=(const VideoMode& left, const VideoMode& right);

} // namespace lava
