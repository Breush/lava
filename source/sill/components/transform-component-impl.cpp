#include "./transform-component-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace lava::sill;

TransformComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
}

void TransformComponent::Impl::postUpdate()
{
    m_changed = false;
}

void TransformComponent::Impl::positionAdd(const glm::vec3& delta)
{
    m_transform = glm::translate(m_transform, delta);
    m_changed = true;
}
