#pragma once

#include <glm/vec3.hpp>

namespace lava::dike {
    // @todo All these should be pure virtual
    class IRigidBody {
    public:
        virtual ~IRigidBody() = default;

        void mass(float mass);

        virtual glm::vec3 translation() const = 0;
        virtual void translate(const glm::vec3& delta) = 0;
    };
}
