#include <lava/sill/components/i-ui-component.hpp>

#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>

using namespace lava::sill;

IUiComponent::IUiComponent(Entity& entity)
    : IComponent(entity)
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    entity.engine().ui().registerUiComponent(*this);
}

IUiComponent::~IUiComponent()
{
    m_entity.engine().ui().unregisterUiComponent(*this);
}

// ----- UI manager interaction

bool IUiComponent::checkHovered(const glm::ivec2& mousePosition)
{
    const auto& position = m_transformComponent.translation2d();
    bool hovered = (mousePosition.x >= position.x - m_extent.x / 2.f &&
                    mousePosition.x <= position.x + m_extent.x / 2.f &&
                    mousePosition.y >= position.y - m_extent.y / 2.f &&
                    mousePosition.y <= position.y + m_extent.y / 2.f);
    this->hovered(hovered);
    return hovered;
}
