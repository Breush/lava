#include "./inspection-panel.hpp"

#include <lava/chamber/string-tools.hpp>

using namespace lava;
using namespace lava::chamber;

namespace {
    void reflowWidgets(GameState& gameState)
    {
        constexpr float xMargin = 4.f;
        constexpr float yMargin = 4.f;
        float y = yMargin;

        for (const auto& widget : gameState.ui.inspectionPanel.widgets) {
            auto kind = widget.kind();
            auto& entity = *widget.entity();
            if (kind == UiWidgetKind::TextEntry) {
                auto& uiComponent = entity.get<sill::UiTextEntryComponent>();
                auto& extent = uiComponent.extent();
                entity.get<sill::TransformComponent>().translation2d({xMargin + extent.x / 2.f, y + extent.y / 2.f});
                y += yMargin + extent.y;
            }
            else if (kind == UiWidgetKind::ToggleButton) {
                auto& uiComponent = entity.get<sill::UiButtonComponent>();
                auto& extent = uiComponent.extent();
                entity.get<sill::TransformComponent>().translation2d({xMargin + extent.x / 2.f, y + extent.y / 2.f});
                y += yMargin + extent.y;
            }
            else if (kind == UiWidgetKind::Select) {
                auto& uiComponent = entity.get<sill::UiSelectComponent>();
                auto& extent = uiComponent.extent();
                entity.get<sill::TransformComponent>().translation2d({xMargin + extent.x / 2.f, y + extent.y / 2.f});
                y += yMargin + extent.y;
            }
        }
    }
}

void ui::inspectObjects(GameState& gameState, const std::vector<Object*>& objects)
{
    // Erase UI
    for (auto entity : gameState.ui.inspectionPanel.entities) {
        gameState.engine->remove(*entity);
    }
    gameState.ui.inspectionPanel.entities.clear();
    gameState.ui.inspectionPanel.widgets.clear();

    if (objects.size() != 1u) return;

    // Create UI
    auto& object = *objects[0u];
    object.uiWidgets(gameState.ui.inspectionPanel.widgets);

    for (auto& widget : gameState.ui.inspectionPanel.widgets) {
        auto kind = widget.kind();
        auto& entity = gameState.engine->make<sill::Entity>("ui.inspection." + widget.id());
        if (kind == UiWidgetKind::TextEntry) {
            auto& textEntry = widget.textEntry();
            auto& uiComponent = entity.make<sill::UiTextEntryComponent>(textEntry.getter());
            uiComponent.onTextChanged([textEntry](const u8string& text) {
                textEntry.setter(text);
            });
        }
        else if (kind == UiWidgetKind::ToggleButton) {
            auto& toggleButton = widget.toggleButton();
            auto& uiComponent = entity.make<sill::UiButtonComponent>(toggleButton.text + ": " + std::to_string(toggleButton.getter()));
            uiComponent.onClicked([toggleButton, &uiComponent]() {
                toggleButton.setter(!toggleButton.getter());
                uiComponent.text(toggleButton.text + ": " + std::to_string(toggleButton.getter()));
            });
            uiComponent.onExtentChanged([&gameState](const glm::vec2& /* extent */) {
                reflowWidgets(gameState);
            });
        }
        else if (kind == UiWidgetKind::Select) {
            auto& select = widget.select();
            std::vector<u8string> options;
            options.reserve(select.options.size());
            for (auto option : select.options) {
                options.emplace_back(std::string(option));
            }
            auto& uiComponent = entity.make<sill::UiSelectComponent>(options, select.getter());
            uiComponent.onIndexChanged([select](uint8_t index, const u8string&) {
                select.setter(index);
            });
        }
        gameState.ui.inspectionPanel.entities.emplace_back(&entity);
        widget.entity(&entity);
    }

    reflowWidgets(gameState);
}
