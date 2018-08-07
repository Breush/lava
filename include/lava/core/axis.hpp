#pragma once

#include <glm/mat3x3.hpp>
#include <lava/core/macros.hpp>

$enum_class(lava, Axis,
            Unknown,   // Unknown axis
            PositiveX, // Positive X
            PositiveY, // Positive Y
            PositiveZ, // Positive Z
            NegativeX, // Negative X
            NegativeY, // Negative Y
            NegativeZ, // Negative Z
);

namespace lava {
    inline glm::vec3 vectorFromAxis(Axis axis)
    {
        switch (axis) {
        case Axis::NegativeX: return glm::vec3(-1.f, 0.f, 0.f);
        case Axis::NegativeY: return glm::vec3(0.f, -1.f, 0.f);
        case Axis::NegativeZ: return glm::vec3(0.f, 0.f, -1.f);
        case Axis::PositiveX: return glm::vec3(1.f, 0.f, 0.f);
        case Axis::PositiveY: return glm::vec3(0.f, 1.f, 0.f);
        case Axis::PositiveZ: return glm::vec3(0.f, 0.f, 1.f);
        default: break;
        }

        return glm::vec3(0.f, 0.f, 0.f);
    }

    inline glm::mat3 rotationMatrixFromAxes(Axis frontAxis, Axis leftAxis, Axis upAxis)
    {
        glm::mat3 rotationMatrix;
        rotationMatrix[0u] = vectorFromAxis(frontAxis);
        rotationMatrix[1u] = vectorFromAxis(leftAxis);
        rotationMatrix[2u] = vectorFromAxis(upAxis);
        return glm::transpose(rotationMatrix);
    }
}
