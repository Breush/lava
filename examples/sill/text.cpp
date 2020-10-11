/**
 * Shows how the text component.
 */

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app;
    auto& engine = app.engine();

    // TextMeshComponent
    {
        auto& entity = engine.make<sill::Entity>();
        auto& textMeshComponent = entity.make<sill::TextMeshComponent>();
        textMeshComponent.text(u8"Hello pretty developer!\n"
                               u8"This is a simple test: ×→ Éŀéþhänt Ðỗdµ ←×→ ✈ሓ𓂀𓅃のᅙᆁ👁 ←×\n"
                               u8"At least, I thought it would be.");
        textMeshComponent.anchor(Anchor::Top);
        textMeshComponent.alignment(Alignment::Center);
    }

    engine.run();

    return EXIT_SUCCESS;
}
