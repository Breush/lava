#include "./VideoModeImpl.hpp"

#include <algorithm>
#include <functional>
#include <lava/crater/VideoMode.hpp>

namespace lava {
    ////////////////////////////////////////////////////////////
    VideoMode::VideoMode()
        : width(0)
        , height(0)
        , bitsPerPixel(0)
    {
    }

    ////////////////////////////////////////////////////////////
    VideoMode::VideoMode(unsigned int modeWidth, unsigned int modeHeight, unsigned int modeBitsPerPixel)
        : width(modeWidth)
        , height(modeHeight)
        , bitsPerPixel(modeBitsPerPixel)
    {
    }

    ////////////////////////////////////////////////////////////
    VideoMode VideoMode::getDesktopMode()
    {
        // Directly forward to the OS-specific implementation
        return priv::VideoModeImpl::getDesktopMode();
    }

    ////////////////////////////////////////////////////////////
    const std::vector<VideoMode>& VideoMode::getFullscreenModes()
    {
        static std::vector<VideoMode> modes;

        // Populate the array on first call
        if (modes.empty()) {
            modes = priv::VideoModeImpl::getFullscreenModes();
            std::sort(modes.begin(), modes.end(), std::greater<VideoMode>());
        }

        return modes;
    }

    ////////////////////////////////////////////////////////////
    bool VideoMode::isValid() const
    {
        const std::vector<VideoMode>& modes = getFullscreenModes();

        return std::find(modes.begin(), modes.end(), *this) != modes.end();
    }

    ////////////////////////////////////////////////////////////
    bool operator==(const VideoMode& left, const VideoMode& right)
    {
        return (left.width == right.width) && (left.height == right.height) && (left.bitsPerPixel == right.bitsPerPixel);
    }

    ////////////////////////////////////////////////////////////
    bool operator!=(const VideoMode& left, const VideoMode& right) { return !(left == right); }

    ////////////////////////////////////////////////////////////
    bool operator<(const VideoMode& left, const VideoMode& right)
    {
        if (left.bitsPerPixel == right.bitsPerPixel) {
            if (left.width == right.width) {
                return left.height < right.height;
            }
            else {
                return left.width < right.width;
            }
        }
        else {
            return left.bitsPerPixel < right.bitsPerPixel;
        }
    }

    ////////////////////////////////////////////////////////////
    bool operator>(const VideoMode& left, const VideoMode& right) { return right < left; }

    ////////////////////////////////////////////////////////////
    bool operator<=(const VideoMode& left, const VideoMode& right) { return !(right < left); }

    ////////////////////////////////////////////////////////////
    bool operator>=(const VideoMode& left, const VideoMode& right) { return !(left < right); }

} // namespace lava
