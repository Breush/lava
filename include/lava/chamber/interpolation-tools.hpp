#pragma once

#include <lava/core/interpolation-type.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace lava::chamber {
    /**
     * Interpolation of values at a specified step.
     *
     * t should be a value between 0 and 1.
     * timeRange should be equal to (timeSteps[step + 1] - timeSteps[step]).
     *
     *  - Step          p(t) = p0
     *                  p0 = values[step]
     *  - Linear        p(t) = p0 + t * (p1 - p0)
     *                  pi = values[step + i]
     *  - CubicSpline   p(t) = (2 t^3 - 3 t^2 + 1) p0 + (t^3 - 2 t^2 + t) m0 + (-2 t^3 + 3 t^2) p1 + (t3 - t2) m1
     *                  pi = values[3 * (step + i) + 1]
     *                  mi = values[3 * (step + i) + 2 * (1 - i)] * timeRange
     */
    glm::vec3 interpolate(const std::vector<glm::vec3>& values, uint32_t step, float t, float timeRange, InterpolationType type);
    glm::quat interpolate(const std::vector<glm::quat>& values, uint32_t step, float t, float timeRange, InterpolationType type);

    /// With this interface, linear interpolation is done between m0 and m1.
    inline float interpolateLinear(float f0, float f1, float t) { return f0 + t * (f1 - f0); }
    inline glm::vec3 interpolateLinear(const glm::vec3& v0, const glm::vec3& v1, float t) { return v0 + t * (v1 - v0); }
    inline glm::vec4 interpolateLinear(const glm::vec4& v0, const glm::vec4& v1, float t) { return v0 + t * (v1 - v0); }
    inline glm::quat interpolateLinear(const glm::quat& q0, const glm::quat& q1, float t) { return glm::slerp(q0, q1, t); }
    glm::mat4 interpolateLinear(const glm::mat4& m0, const glm::mat4& m1, float t);
}
