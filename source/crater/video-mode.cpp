#include <lava/crater/video-mode.hpp>

using namespace lava::crater;

VideoMode::VideoMode(uint16_t width, uint16_t height, uint16_t bitsPerPixel)
    : width(width)
    , height(height)
    , bitsPerPixel(bitsPerPixel)
{
}

VideoMode::VideoMode(const Extent2d& extent, uint16_t bitsPerPixel)
    : width(static_cast<uint16_t>(extent.width))
    , height(static_cast<uint16_t>(extent.height))
    , bitsPerPixel(bitsPerPixel)
{
}
