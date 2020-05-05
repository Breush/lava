#include <lava/chamber/interpolation-tools.hpp>

#include <glm/gtx/matrix_decompose.hpp>

using namespace lava;

namespace {
    float ease(float t, InterpolationEase ease)
    {
        if (t <= 0.f || t >= 1.f) return t;

        if (ease == InterpolationEase::In) {
            return t * t;
        }
        else if (ease == InterpolationEase::Out) {
            return (2.f - t) * t; // 1 - (1 - t)²
        }

        return t;
    }
}

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

    return glm::quat(1.f, 0.f, 0.f, 0.f);
}

Transform chamber::interpolateLinear(const Transform& m0, const Transform& m1, float t)
{
    Transform transform;
    transform.translation = interpolateLinear(m0.translation, m1.translation, t * t);
    transform.scaling = interpolateLinear(m0.scaling, m1.scaling, t);
    transform.rotation = interpolateLinear(m0.rotation, m1.rotation, t);
    return transform;
}

Transform chamber::interpolate(const Transform& m0, const Transform& m1, float t,
                               InterpolationEase translationEase, InterpolationEase rotationEase, InterpolationEase scalingEase)
{
    float tTranslation = ease(t, translationEase);
    float tRotation = ease(t, rotationEase);
    float tScaling = ease(t, scalingEase);

    Transform transform;
    transform.translation = interpolateLinear(m0.translation, m1.translation,tTranslation);
    transform.scaling = interpolateLinear(m0.scaling, m1.scaling, tRotation);
    transform.rotation = interpolateLinear(m0.rotation, m1.rotation, tScaling);
    return transform;
}
