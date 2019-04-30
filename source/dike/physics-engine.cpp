#include <lava/dike/physics-engine.hpp>

#include "./back-engine/bullet/physics-engine-impl.hpp"

using namespace lava::dike;

$pimpl_class(PhysicsEngine);

$pimpl_method(PhysicsEngine, void, update, float, dt);
$pimpl_method(PhysicsEngine, void, gravity, const glm::vec3&, gravity);

void PhysicsEngine::add(std::unique_ptr<IStaticRigidBody>&& staticRigidBody)
{
    m_impl->add(std::move(staticRigidBody));
}

void PhysicsEngine::add(std::unique_ptr<IRigidBody>&& rigidBody)
{
    m_impl->add(std::move(rigidBody));
}

$pimpl_method(PhysicsEngine, void, remove, const IStaticRigidBody&, rigidBody);
$pimpl_method(PhysicsEngine, void, remove, const IRigidBody&, rigidBody);
