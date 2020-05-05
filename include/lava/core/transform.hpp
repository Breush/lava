#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <sstream>

namespace lava {
    /**
     * A basic transform, stored as TRS.
     *
     * Rationale is that, sometimes, we don't want to use glm::mat4 directly
     * if this is just for affine transforms.
     *
     *  @note Non-uniform scaling is not allowed:
     *        this would not allow this representation to fail for
     *        inverses or compositions (both might add shear).
     */
    struct Transform {
        glm::vec3 translation = {0.f, 0.f, 0.f};
        glm::quat rotation = {1.f, 0.f, 0.f, 0.f};
        float scaling = 1.f;

        glm::mat4 matrix() const {
            glm::mat4 matrix(rotation);
            matrix *= glm::mat4(scaling);
            matrix[3][0] = translation.x;
            matrix[3][1] = translation.y;
            matrix[3][2] = translation.z;
            matrix[3][3] = 1.f;
            return matrix;
        }

        Transform inverse() const {
            Transform result;
            result.rotation = glm::inverse(rotation);
            if (scaling == 0.f) {
                result.scaling = 0.f;
                result.translation = -result.rotation * translation;
            } else {
                result.scaling = 1.f / scaling;
                result.translation = -(result.rotation * translation) * result.scaling;
            }
            return result;
        }

        Transform& operator*=(const Transform& rhs) {
            translation += rotation * (scaling * rhs.translation);
            scaling *= rhs.scaling;
            rotation *= rhs.rotation;
            return *this;
        }

        Transform operator*(const Transform& rhs) const {
            return Transform(*this) *= rhs;
        }

        std::string to_string() const {
            std::stringstream s;
            s << "[T(" << translation.x << "," << translation.y << "," << translation.z << ") ";
            s << "R(" << rotation.w << "," << rotation.x << "," << rotation.y << "," << rotation.z << ") ";
            s << "S(" << scaling << ")]";
            return s.str();
        }
    };
}
