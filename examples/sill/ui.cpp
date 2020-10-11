/**
 * Shows how to use the UI.
 */

#include "./ashe.hpp"

#include <iostream>

using namespace lava;

int main(void)
{
    ashe::Application app;
    auto& engine = app.engine();

    // @todo Would be great to be able to position objects through
    // a UiComponent which accepts relative coordinates.
    auto screenExtent = engine.camera2d().extent();

    // Button
    {
        static std::vector<u8string> texts = {
            u8"I am a button!",
            u8"Don't do that...",
            u8"Please stop!",
            u8"I am going to cry...",
            u8"I AM going to cry...",
            u8"I AM GOING TO cry...",
            u8"I AM GOING TO CRY!",
            u8"...", ":'(", "...",
        };

        auto& buttonEntity = engine.make<sill::Entity>("demo.button");
        auto& buttonComponent = buttonEntity.make<sill::UiButtonComponent>(texts[0u]);
        buttonComponent.onClicked([&buttonComponent]() {
            static auto textIndex = 0u;
            textIndex = (textIndex + 1u) % texts.size();
            buttonComponent.text(texts[textIndex]);
        });

        buttonEntity.get<sill::TransformComponent>().translation2d({screenExtent.width / 2.f,
                                                                    screenExtent.height / 2.f});
    }

    // TextEntry
    {
        auto& textEntryEntity = engine.make<sill::Entity>("demo.text-entry");
        auto& textEntryComponent = textEntryEntity.make<sill::UiTextEntryComponent>();
        textEntryComponent.onTextChanged([](const u8string& text) {
            std::cout << "TextEntry: " << text << std::endl;
        });

        textEntryEntity.get<sill::TransformComponent>().translation2d({screenExtent.width / 2.f,
                                                                       screenExtent.height / 2.f + 32u});
    }

    // Select
    {
        std::vector<u8string> options;
        options.emplace_back(u8"Hello!");
        options.emplace_back(u8"That's a lot...");
        options.emplace_back(u8"... of options to choose from!");

        auto& selectEntity = engine.make<sill::Entity>("demo.select");
        auto& selectComponent = selectEntity.make<sill::UiSelectComponent>(options, 0);
        selectComponent.onIndexChanged([](uint8_t index, const u8string& text) {
            std::cout << "Select: " << (uint32_t)index << " -> " << text << std::endl;
        });

        selectEntity.get<sill::TransformComponent>().translation2d({screenExtent.width / 2.f,
                                                                    screenExtent.height / 2.f + 64u});
    }

    engine.run();

    return EXIT_SUCCESS;
}
