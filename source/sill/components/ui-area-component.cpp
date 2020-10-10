#include <lava/sill/components/ui-area-component.hpp>

#include <lava/sill/components/transform-component.hpp>

using namespace lava::sill;

UiAreaComponent::UiAreaComponent(Entity& entity)
    : UiAreaComponent(entity, {0.f, 0.f})
{
}

UiAreaComponent::UiAreaComponent(Entity& entity, const glm::vec2& extent)
    : IUiComponent(entity)
{
    m_extent = extent;
    m_hoveringExtent = m_extent;

    updateFromAnchor();
}

// ----- IUiComponent

void UiAreaComponent::verticallyScrolled(float delta, bool& propagate)
{
    propagate = false;

    glm::vec2 offset{0.f, -m_scrollSensibility * delta};

    if (m_scrollOffset.y + offset.y > m_scrollMaxOffset.y) {
        offset.y = m_scrollMaxOffset.y - m_scrollOffset.y;
    }
    if (m_scrollOffset.y + offset.y < m_scrollMinOffset.y) {
        offset.y = m_scrollMinOffset.y - m_scrollOffset.y;
    }

    m_transformComponent.translate2d(-offset);
    m_hoveringOffset += offset;

    m_scrollOffset += offset;
}

// ----- Configuration

void UiAreaComponent::scrollOffset(const glm::vec2& scrollOffset)
{
    m_transformComponent.translate2d(m_scrollOffset);
    m_hoveringOffset -= m_scrollOffset;

    m_scrollOffset = scrollOffset;
    if (m_scrollOffset.y > m_scrollMaxOffset.y) {
        m_scrollOffset.y = m_scrollMaxOffset.y;
    }
    if (m_scrollOffset.y < m_scrollMinOffset.y) {
        m_scrollOffset.y = m_scrollMinOffset.y;
    }

    m_hoveringOffset += m_scrollOffset;
    m_transformComponent.translate2d(-m_scrollOffset);
}

// ----- Internal

void UiAreaComponent::updateFromAnchor()
{
    m_transformComponent.translate2d(m_scrollOffset);

    m_hoveringOffset.x = 0.f;
    m_hoveringOffset.y = 0.f;

    switch (m_anchor) {
        case Anchor::TopLeft:
            m_hoveringOffset = {m_hoveringExtent.x / 2.f, m_hoveringExtent.y / 2.f};
            break;
        case Anchor::Top:
            m_hoveringOffset = {0.f, m_hoveringExtent.y / 2.f};
            break;
        case Anchor::TopRight:
            m_hoveringOffset = {-m_hoveringExtent.x / 2.f, m_hoveringExtent.y / 2.f};
            break;
        case Anchor::Left:
            m_hoveringOffset = {m_hoveringExtent.x / 2.f, 0.f};
            break;
        case Anchor::Center:
            m_hoveringOffset = {0.f, 0.f};
            break;
        case Anchor::Right:
            m_hoveringOffset = {-m_hoveringExtent.x / 2.f, 0.f};
            break;
        case Anchor::BottomLeft:
            m_hoveringOffset = {m_hoveringExtent.x / 2.f, -m_hoveringExtent.y / 2.f};
            break;
        case Anchor::Bottom:
            m_hoveringOffset = {0.f, -m_hoveringExtent.y / 2.f};
            break;
        case Anchor::BottomRight:
            m_hoveringOffset = {-m_hoveringExtent.x / 2.f, -m_hoveringExtent.y / 2.f};
            break;
    }

    m_transformComponent.translate2d(-m_scrollOffset);
    m_hoveringOffset += m_scrollOffset;
}
