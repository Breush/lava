#pragma once

#include <glm/vec3.hpp>
#include <memory>

namespace lava::dike {
    class RigidBody;
}

namespace lava::dike {
    class PhysicsEngine {
    public:
        PhysicsEngine();
        ~PhysicsEngine();

        void update(float dt);

        void gravity(const glm::vec3& gravity);

        /// Allows the enabled/disabled the physics engine.
        void enabled(bool enabled) { m_enabled = enabled; }

        /**
         * @name Makers
         * Make a new resource and add it to the engine.
         *
         * Arguments will be forwarded to the constructor.
         * Any resource that match an adder (see below) can be made.
         *
         * ```
         * auto& rigidBody = engine.make<RigidBody>(); // Its lifetime is now managed by the engine.
         * ```
         */
        /// @{
        /// Make a new resource directly.
        template <class T, class... Arguments>
        T& make(Arguments&&... arguments);
        /// @}

        /**
         * @name Adders
         * Add a resource that has already been created.
         *
         * Its ownership goes to the engine.
         * For convenience, you usually want to use makers (see above).
         */
        /// @{
        void add(std::unique_ptr<RigidBody>&& rigidBody);

        void remove(const RigidBody& rigidBody);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }
        const Impl& impl() const { return *m_impl; }

    private:
        Impl* m_impl = nullptr;

        bool m_enabled = true;
    };
}

#include <lava/dike/physics-engine.inl>
