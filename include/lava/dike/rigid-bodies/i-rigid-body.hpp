#pragma once

#include <glm/vec3.hpp>

namespace lava::dike {
    // @todo All these should be pure virtual
    class IRigidBody {
    public:
        virtual ~IRigidBody() = default;

        void mass(float mass);

        /// Whether the rigid body colliders should be considered.
        virtual bool enabled() const = 0;
        virtual void enabled(bool enabled) = 0;

        virtual const glm::mat4& transform() const = 0;
        virtual void transform(const glm::mat4& transform) = 0;

        virtual glm::vec3 translation() const = 0;
        virtual void translate(const glm::vec3& delta) = 0;
    };
}
