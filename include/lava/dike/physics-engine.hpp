#pragma once

#include <lava/dike/rigid-bodies/i-rigid-body.hpp>
#include <lava/dike/static-rigid-bodies/i-static-rigid-body.hpp>

#include <glm/vec3.hpp>
#include <memory>

namespace lava::dike {
    class PhysicsEngine {
    public:
        PhysicsEngine();
        ~PhysicsEngine();

        void update(float dt);

        void gravity(const glm::vec3& gravity);

        /**
        * @name Makers
        * Make a new resource and add it to the engine.
        *
        * Arguments will be forwarded to the constructor.
        * Any resource that match an adder (see below) can be made.
        *
        * ```
        * auto& scene = engine.make<SphereRigidBody>(); // Its lifetime is now managed by the engine.
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
        void add(std::unique_ptr<IStaticRigidBody>&& rigidBody);
        void add(std::unique_ptr<IRigidBody>&& rigidBody);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }
        const Impl& impl() const { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}

#include <lava/dike/physics-engine.inl>
