#pragma once

#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
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
            result.rotation = glm::conjugate(rotation);
            if (scaling == 0.f) {
                result.scaling = 0.f;
                result.translation = -result.rotate(translation);
            } else {
                result.scaling = 1.f / scaling;
                result.translation = -result.scaling * result.rotate(translation);
            }
            return result;
        }

        glm::vec3 rotate(const glm::vec3& v) const {
            return rotation * v;
        }

        Transform& operator*=(const Transform& rhs) {
            translation += rotate(scaling * rhs.translation);
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

    /**
     * A basic transform, stored as TRS, for 2D.
     */
    struct Transform2d {
        glm::vec2 translation = {0.f, 0.f};
        float rotation = 0.f;
        float scaling = 1.f;

        glm::mat3 matrix() const {
            const auto c = scaling * cos(rotation);
            const auto s = scaling * sin(rotation);
            return glm::mat3( c, s, 0.f,
                             -s, c, 0.f,
                             translation.x, translation.y, 1.f);
        }

        Transform2d inverse() const {
            Transform2d result;
            result.rotation = -rotation;
            if (scaling == 0.f) {
                result.scaling = 0.f;
                result.translation = -result.rotate(translation);
            } else {
                result.scaling = 1.f / scaling;
                result.translation = -result.scaling * result.rotate(translation);
            }
            return result;
        }

        glm::vec2 rotate(const glm::vec2& v) const {
            const auto c = cos(rotation);
            const auto s = sin(rotation);

            glm::vec2 result;
            result.x = v.x * c - v.y * s;
            result.y = v.x * s + v.y * c;
            return result;
        }

        Transform2d& operator*=(const Transform2d& rhs) {
            translation += rotate(scaling * rhs.translation);
            scaling *= rhs.scaling;
            rotation += rhs.rotation;
            return *this;
        }

        Transform2d operator*(const Transform2d& rhs) const {
            return Transform2d(*this) *= rhs;
        }

        std::string to_string() const {
            std::stringstream s;
            s << "[T(" << translation.x << "," << translation.y << ") ";
            s << "R(" << rotation << ") ";
            s << "S(" << scaling << ")]";
            return s.str();
        }
    };
}
