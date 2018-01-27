#pragma once

#include <glm/vec3.hpp>

namespace lava::dike {
    // @todo All these should be pure virtual
    class IRigidBody {
    public:
        virtual ~IRigidBody() = default;

        void mass(float mass);

        virtual glm::vec3 position() const = 0;
        virtual void positionAdd(const glm::vec3& delta) = 0;
    };
}
