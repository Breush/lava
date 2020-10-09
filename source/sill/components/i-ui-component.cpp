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
    if (!m_entity.engine().destroying()) {
        m_entity.engine().ui().unregisterUiComponent(*this);
    }
}

// ----- UI manager interaction

glm::vec2 IUiComponent::topLeftRelativePosition(const glm::ivec2& mousePosition) const
{
    auto position = m_transformComponent.worldTransform2d().translation;
    return glm::vec2(mousePosition) - position + m_extent / 2.f;
}

bool IUiComponent::checkHovered(const glm::ivec2& mousePosition)
{
    auto position = m_transformComponent.worldTransform2d().translation;
    position += m_hoveringOffset;
    bool hovered = (mousePosition.x >= position.x - m_hoveringExtent.x / 2.f &&
                    mousePosition.x <= position.x + m_hoveringExtent.x / 2.f &&
                    mousePosition.y >= position.y - m_hoveringExtent.y / 2.f &&
                    mousePosition.y <= position.y + m_hoveringExtent.y / 2.f);
    this->hovered(hovered);
    return hovered;
}
