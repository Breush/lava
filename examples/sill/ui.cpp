/**
 * Shows how to use the UI.
 */

#include "./ashe.hpp"

#include <iostream>
#include <lava/magma/camera.hpp>

using namespace lava;

int main(void)
{
    ashe::Application app;
    auto& engine = app.engine();

    // Button
    {
        static std::vector<std::wstring> texts = {
            L"I am a button!",
            L"Don't do that...",
            L"Please stop!",
            L"I am going to cry...",
            L"I AM going to cry...",
            L"I AM GOING TO cry...",
            L"I AM GOING TO CRY!",
            L"...", L":'(", L"...",
        };

        auto& buttonEntity = engine.make<sill::GameEntity>("demo.button");
        auto& buttonComponent = buttonEntity.make<sill::UiButtonComponent>(texts[0u]);
        buttonComponent.onClicked([&buttonComponent]() {
            static auto textIndex = 0u;
            textIndex = (textIndex + 1u) % texts.size();
            buttonComponent.text(texts[textIndex]);
        });

        // @todo Would be great to be able to position objects through
        // a UiComponent which accepts relative coordinates.
        auto screenExtent = engine.camera2d().extent();
        buttonEntity.get<sill::TransformComponent>().translation2d({screenExtent.width / 2.f, screenExtent.height / 2.f});
    }

    engine.run();

    return EXIT_SUCCESS;
}
