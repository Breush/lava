#pragma once

#include "../game-state.hpp"

namespace ui {
    enum class ChangeKind {
        Unknown,
        Removing,
        Adding,
    };

    void hideEntitiesPanel(GameState& gameState);

    void updateEntitiesPanel(GameState& gameState, const std::vector<std::unique_ptr<Object>>& objects, ChangeKind changeKind = ChangeKind::Unknown);
    void updateEntitiesPanel(GameState& gameState, const std::vector<Object*>& objects, ChangeKind changeKind = ChangeKind::Unknown);
}
