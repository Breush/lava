#include <lava/dike/physics-engine.hpp>

#include "./back-engine/bullet/physics-engine-impl.hpp"

using namespace lava::dike;

$pimpl_class(PhysicsEngine);

void PhysicsEngine::update(float dt)
{
    if (!m_enabled) return;

    m_impl->update(dt);
}

$pimpl_method(PhysicsEngine, void, gravity, const glm::vec3&, gravity);

void PhysicsEngine::add(std::unique_ptr<RigidBody>&& rigidBody)
{
    m_impl->add(std::move(rigidBody));
}

$pimpl_method(PhysicsEngine, void, remove, const RigidBody&, rigidBody);
