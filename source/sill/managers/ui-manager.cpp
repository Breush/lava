#include <lava/sill/managers/ui-manager.hpp>

#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>

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

    auto hoveredUiComponentIt = std::find(m_hoveredUiComponents.begin(), m_hoveredUiComponents.end(), &uiComponent);
    if (hoveredUiComponentIt != m_hoveredUiComponents.end()) {
        m_hoveredUiComponents.erase(hoveredUiComponentIt);
    }

    if (m_draggedUiComponent == &uiComponent) {
        m_draggedUiComponent = nullptr;
    }
}

// ----- Called by game engine

void UiManager::handleEvent(WsEvent& event, bool& propagate)
{
    if (event.type == WsEventType::MouseMoved) {
        m_mousePosition.x = event.mouseMove.x;
        m_mousePosition.y = event.mouseMove.y;

        if (m_draggedUiComponent) {
            propagate = false;
            return;
        }

        // Check if we're hovering anything
        // @note Will update all hovered components,
        // setting false to no-more hovered and keeping true
        // if already hovered.
        m_hoveredUiComponents.clear();
        for (auto uiComponent : m_uiComponents) {
            if (uiComponent->checkHovered(m_mousePosition)) {
                m_hoveredUiComponents.emplace_back(uiComponent);
            }
        }
    }
    else if (event.type == WsEventType::MouseButtonPressed &&
             event.mouseButton.which == MouseButton::Left) {
        for (auto uiComponent : m_hoveredUiComponents) {
            uiComponent->dragStart(m_mousePosition, propagate);

            // If non propagating, we go in dragging state.
            if (!propagate) {
                m_draggedUiComponent = uiComponent;
                break;
            }
        }
    }
    else if (event.type == WsEventType::MouseButtonReleased &&
             event.mouseButton.which == MouseButton::Left) {
        if (!m_draggedUiComponent) return;

        m_draggedUiComponent->dragEnd(m_mousePosition);
        m_draggedUiComponent = nullptr;
    }
    else if (event.type == WsEventType::MouseWheelScrolled) {
        if (event.mouseWheel.which == MouseWheel::Vertical) {
            for (auto uiComponent : m_hoveredUiComponents) {
                uiComponent->verticallyScrolled(event.mouseWheel.delta, propagate);
                if (!propagate) break;
            }
        }
    }
    else if (event.type == WsEventType::KeyPressed) {
        // @todo :UiFocus Have focused component instead
        for (auto uiComponent : m_hoveredUiComponents) {
            uiComponent->textEntered(event.key.which, event.key.code, propagate);
            if (!propagate) break;
        }
    }
}
