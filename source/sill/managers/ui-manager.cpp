#include <lava/sill/managers/ui-manager.hpp>

#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

// @todo :Refactor Should be UiComponent when we grow in similar components.
#include <lava/sill/components/ui-button-component.hpp>

using namespace lava;
using namespace lava::sill;

UiManager::UiManager(GameEngine& engine)
    : m_engine(engine)
{
}

void UiManager::registerEntity(GameEntity& entity)
{
    m_entities.emplace_back(&entity);
}

void UiManager::unregisterEntity(GameEntity& entity)
{
    auto entityIt = std::find(m_entities.begin(), m_entities.end(), &entity);
    m_entities.erase(entityIt);
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
        m_hoveredEntity = nullptr;
        for (auto entity : m_entities) {
            auto& uiComponent = entity->get<UiButtonComponent>();
            if (!m_hoveredEntity) {
                if (uiComponent.checkHovered(m_mousePosition)) {
                    m_hoveredEntity = entity;
                }
            }
            else {
                // Something above is hovered.
                uiComponent.hovered(false);
            }
        }
    }
    else if (event.type == WsEventType::MouseButtonPressed &&
             event.mouseButton.which == MouseButton::Left) {
        if (!m_hoveredEntity) return;

        // If anything hovered, events should not propagate.
        propagate = false;

        m_dragging = true;
        m_hoveredEntity->get<UiButtonComponent>().dragStart(m_mousePosition);
    }
    else if (event.type == WsEventType::MouseButtonReleased &&
             event.mouseButton.which == MouseButton::Left) {
        if (!m_dragging) return;

        m_dragging = false;
        auto& uiComponent = m_hoveredEntity->get<UiButtonComponent>();
        uiComponent.dragEnd(m_mousePosition);
    }
}


