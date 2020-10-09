#include "./entities-panel.hpp"

#include "../editor.hpp"

#include <lava/chamber/string-tools.hpp>

using namespace lava;
using namespace lava::chamber;

namespace {
    void doUpdateEntitiesPanel(GameState& gameState, const std::vector<Object*>& objects, ui::ChangeKind changeKind)
    {
        constexpr float panelWidth = 200.f; // @fixme Hard-coded...
        const auto& cameraExtent = gameState.camera.component->extent();

        auto& entities = gameState.ui.entitiesPanel.entities;
        auto& areaComponent = gameState.ui.entitiesPanel.areaComponent;

        if (changeKind == ui::ChangeKind::Unknown) {
            ui::hideEntitiesPanel(gameState);
            gameState.ui.entitiesPanel.areaComponent = nullptr;
        }

        // Create scrollable area if needed
        if (!gameState.ui.entitiesPanel.areaComponent) {
            auto& areaEntity = gameState.engine->make<sill::Entity>("ui.entities.area");
            areaComponent = &areaEntity.make<sill::UiAreaComponent>(glm::vec2{panelWidth, cameraExtent.height});
            areaComponent->anchor(Anchor::Top);
            areaEntity.get<sill::TransformComponent>().translation2d({cameraExtent.width - panelWidth / 2.f, 0.f});
            entities.emplace_back(&areaEntity);
        }

        if (changeKind == ui::ChangeKind::Removing) {
            // Remove entities that are no more in the new objects
            auto it = std::remove_if(entities.begin(), entities.end(), [&gameState, &objects](sill::Entity* entity) {
                if (!entity->has<sill::UiLabelComponent>()) return false;
                auto labelObject = entity->get<sill::UiLabelComponent>().userDataAs<Object*>();
                if (std::find(objects.begin(), objects.end(), labelObject) != objects.end()) return false;
                gameState.engine->remove(*entity);
                return true;
            });
            entities.erase(it, entities.end());
        }
        else {
            for (auto object : objects) {
                // Add entities only for the new objects
                if (changeKind == ui::ChangeKind::Adding) {
                    auto it = std::find_if(entities.begin(), entities.end(), [&gameState, object](const sill::Entity* entity) {
                        if (!entity->has<sill::UiLabelComponent>()) return false;
                        return entity->get<sill::UiLabelComponent>().userDataAs<Object*>() == object;
                    });
                    if (it != entities.end()) continue;
                }

                auto& entity = gameState.engine->make<sill::Entity>("ui.entities.label");
                auto& labelComponent = entity.make<sill::UiLabelComponent>(utf8to16("[" + object->kind() + "] " + object->name()));
                labelComponent.userData(object);
                labelComponent.onClicked([&gameState, object]() {
                    selectObject(gameState, object, false);
                });
                entities.emplace_back(&entity);
                areaComponent->entity().addChild(entity);
            }
        }

        // Placing all entities
        // @fixme Have a hierarchical view based on entity parenting
        float y = 0.f;
        glm::vec2 labelExtent;

        for (auto entity : entities) {
            if (!entity->has<sill::UiLabelComponent>()) continue;
            auto& uiComponent = entity->get<sill::UiLabelComponent>();
            labelExtent = uiComponent.extent();
            entity->get<sill::TransformComponent>().translation2d({0.f, y + labelExtent.y / 2.f});
            y += labelExtent.y;
        }

        areaComponent->maxOffset({INFINITY, y - cameraExtent.height});
        areaComponent->scrollSensibility(labelExtent.y);
    }
}

void ui::hideEntitiesPanel(GameState& gameState)
{
    for (auto entity : gameState.ui.entitiesPanel.entities) {
        gameState.engine->remove(*entity);
    }
    gameState.ui.entitiesPanel.entities.clear();
}

void ui::updateEntitiesPanel(GameState& gameState, const std::vector<std::unique_ptr<Object>>& objects, ChangeKind changeKind)
{
    std::vector<Object*> rawObjects;
    rawObjects.reserve(objects.size());
    for (const auto& object : objects) {
        rawObjects.emplace_back(object.get());
    }
    doUpdateEntitiesPanel(gameState, rawObjects, changeKind);
}

void ui::updateEntitiesPanel(GameState& gameState, const std::vector<Object*>& objects, ChangeKind changeKind)
{
    doUpdateEntitiesPanel(gameState, objects, changeKind);
}
