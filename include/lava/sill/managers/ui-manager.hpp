#pragma once

#include <glm/vec2.hpp>
#include <lava/core/ws-event.hpp>
#include <vector>

namespace lava::sill {
    class GameEngine;
    class GameEntity;
}

namespace lava::sill {
    /**
     * Will update UI entities.
     */
    class UiManager {
    public:
        UiManager(GameEngine& engine);

        /// Add or remove an entity to the UI.
        void registerEntity(GameEntity& entity);
        void unregisterEntity(GameEntity& entity);

        // ----- Called by game engine

        void handleEvent(WsEvent& event, bool& propagate);

    private:
        GameEngine& m_engine;

        std::vector<GameEntity*> m_entities;

        // State
        glm::ivec2 m_mousePosition;
        GameEntity* m_hoveredEntity = nullptr;
        bool m_dragging = false;
    };
}
