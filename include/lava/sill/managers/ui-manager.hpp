#pragma once

#include <glm/vec2.hpp>
#include <lava/core/ws-event.hpp>
#include <vector>

namespace lava::sill {
    class GameEngine;
    class IUiComponent;
}

namespace lava::sill {
    /**
     * Will update UI entities.
     */
    class UiManager {
    public:
        UiManager(GameEngine& engine);

        /// Add or remove a component to the UI.
        void registerUiComponent(IUiComponent& uiComponent);
        void unregisterUiComponent(IUiComponent& uiComponent);

        // ----- Called by game engine

        void handleEvent(WsEvent& event, bool& propagate);

    private:
        GameEngine& m_engine;

        // Registered entities
        std::vector<IUiComponent*> m_uiComponents;

        // State
        glm::ivec2 m_mousePosition;
        std::vector<IUiComponent*> m_hoveredUiComponents; // Sort by depth, from closest to furthest
        IUiComponent* m_draggedUiComponent = nullptr;
    };
}
