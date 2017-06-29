#include <lava/crater/video-mode.hpp>

#include <algorithm>
#include <functional>

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
}
