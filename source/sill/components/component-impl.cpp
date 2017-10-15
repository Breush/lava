#include "./component-impl.hpp"

using namespace lava::sill;

ComponentImpl::ComponentImpl(GameEntity& entity)
    : m_entity(entity.impl())
{
}
