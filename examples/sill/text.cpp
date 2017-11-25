/**
 * Shows how the text component.
 */

#include <lava/sill.hpp>

using namespace lava;

int main(void)
{
    sill::GameEngine engine;

    // TextMeshComponent
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& textMeshComponent = entity.make<sill::TextMeshComponent>();
        textMeshComponent.text(L"Hello pretty developer!\n"
                               L"This is a simple test: ×→ Éŀéþhänt Ðỗdµ ←×→ ✈ሓ𓂀𓅃のᅙᆁ ←×\n"
                               L"At least, I thought it would be.");
        textMeshComponent.verticalAnchor(Anchor::START);
        textMeshComponent.alignment(Alignment::CENTER);
    }

    engine.run();

    return EXIT_SUCCESS;
}
