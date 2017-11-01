#include <lava/chamber/math/trigonometry.hpp>

#include <cmath>

using namespace lava::chamber;

float math::cos(float t)
{
    return std::cos(t);
}

float math::sin(float t)
{
    return std::sin(t);
}

float math::atan(float x, float y)
{
    return std::atan2(x, y);
}
