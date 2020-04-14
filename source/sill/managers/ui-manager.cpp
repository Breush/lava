#include <lava/sill/managers/ui-manager.hpp>

#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

#include <lava/sill/components/i-ui-component.hpp>

using namespace lava;
using namespace lava::sill;

UiManager::UiManager(GameEngine& engine)
    : m_engine(engine)
{
}

void UiManager::registerUiComponent(IUiComponent& uiComponent)
{
    m_uiComponents.emplace_back(&uiComponent);
}

void UiManager::unregisterUiComponent(IUiComponent& uiComponent)
{
    auto uiComponentIt = std::find(m_uiComponents.begin(), m_uiComponents.end(), &uiComponent);
    m_uiComponents.erase(uiComponentIt);

    if (m_hoveredUiComponent == &uiComponent) {
        m_hoveredUiComponent = nullptr;
        m_dragging = false;
    }
}

// ----- Called by game engine

void UiManager::handleEvent(WsEvent& event, bool& propagate)
{
    if (event.type == WsEventType::MouseMoved) {
        m_mousePosition.x = event.mouseMove.x;
        m_mousePosition.y = event.mouseMove.y;

        if (m_dragging) {
            propagate = false;
            return;
        }

        // Check if we're hovering anything
        m_hoveredUiComponent = nullptr;
        for (auto uiComponent : m_uiComponents) {
            if (!m_hoveredUiComponent) {
                if (uiComponent->checkHovered(m_mousePosition)) {
                    m_hoveredUiComponent = uiComponent;
                }
            }
            else {
                // Something above is hovered.
                uiComponent->hovered(false);
            }
        }
    }
    else if (event.type == WsEventType::MouseButtonPressed &&
             event.mouseButton.which == MouseButton::Left) {
        if (!m_hoveredUiComponent) return;
        m_hoveredUiComponent->dragStart(m_mousePosition, propagate);

        // If non propagating, we go in dragging state.
        m_dragging = !propagate;
    }
    else if (event.type == WsEventType::MouseButtonReleased &&
             event.mouseButton.which == MouseButton::Left) {
        if (!m_dragging) return;

        m_dragging = false;
        m_hoveredUiComponent->dragEnd(m_mousePosition);
    }
    else if (event.type == WsEventType::KeyPressed) {
        if (!m_hoveredUiComponent) return; // @todo :UiFocus Have focused component instead
        m_hoveredUiComponent->textEntered(event.key.which, event.key.code, propagate);
    }
}


