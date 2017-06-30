#include <lava/crater/video-mode.hpp>

using namespace lava;

VideoMode::VideoMode(uint16_t width, uint16_t height, uint16_t bitsPerPixel)
    : width(width)
    , height(height)
    , bitsPerPixel(bitsPerPixel)
{
}
