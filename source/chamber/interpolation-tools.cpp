#include <lava/chamber/interpolation-tools.hpp>

using namespace lava;

glm::vec3 chamber::interpolate(const std::vector<glm::vec3>& values, uint32_t step, float t, float timeRange,
                               InterpolationType type)
{
    if (type == InterpolationType::Linear) {
        if (t >= 1.f) return values[step];
        const auto& previousValue = values[step];
        const auto& nextValue = values[step + 1u];
        return glm::mix(previousValue, nextValue, t);
    }
    else if (type == InterpolationType::Step) {
        if (t >= 1.f) return values[step];
        return values[step];
    }
    else if (type == InterpolationType::CubicSpline) {
        if (t >= 1.f) return values[3 * step + 1u];
        // p(t) = (2t^3 - 3t^2 + 1)p0 + (t^3 - 2t^2 + t)m0 + (-2t^3 + 3t^2)p1 + (t^3 - t^2)m1
        const auto t2 = t * t;
        const auto t3 = t * t * t;
        const auto& p0 = values[3 * step + 1u];
        const auto& m0 = values[3 * step + 2u] * timeRange;
        const auto& m1 = values[3 * (step + 1u)] * timeRange;
        const auto& p1 = values[3 * (step + 1u) + 1u];
        return (2 * t3 - 3 * t2 + 1) * p0 + (t3 - 2 * t2 + t) * m0 + (-2 * t3 + 3 * t2) * p1 + (t3 - t2) * m1;
    }

    return glm::vec3(0.f);
}

glm::quat chamber::interpolate(const std::vector<glm::quat>& values, uint32_t step, float t, float timeRange,
                               InterpolationType type)
{
    if (type == InterpolationType::Linear) {
        if (t >= 1.f) return values[step];
        auto& previousValue = values[step];
        auto& nextValue = values[step + 1u];
        return glm::slerp(previousValue, nextValue, t);
    }
    else if (type == InterpolationType::Step) {
        if (t >= 1.f) return values[step];
        return values[step];
    }
    else if (type == InterpolationType::CubicSpline) {
        if (t >= 1.f) return values[3 * step + 1u];
        // p(t) = (2t^3 - 3t^2 + 1)p0 + (t^3 - 2t^2 + t)m0 + (-2t^3 + 3t^2)p1 + (t^3 - t^2)m1
        const auto t2 = t * t;
        const auto t3 = t * t * t;
        const auto& p0 = values[3 * step + 1u];
        const auto& m0 = values[3 * step + 2u] * timeRange;
        const auto& m1 = values[3 * (step + 1u)] * timeRange;
        const auto& p1 = values[3 * (step + 1u) + 1u];
        auto value = (2 * t3 - 3 * t2 + 1) * p0 + (t3 - 2 * t2 + t) * m0 + (-2 * t3 + 3 * t2) * p1 + (t3 - t2) * m1;
        return glm::normalize(value);
    }

    return glm::quat(0.f, 0.f, 0.f, 1.f);
}
