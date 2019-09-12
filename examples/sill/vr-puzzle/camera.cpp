#include "./camera.hpp"

using namespace lava;

void setupCamera(GameState& gameState)
{
    // Initializing inputs
    auto& engine = *gameState.engine;
    auto& input = engine.input();

    input.bindAxis("main-x", InputAxis::MouseX);
    input.bindAxis("main-y", InputAxis::MouseY);
    input.bindAxis("zoom", InputAxis::MouseWheelVertical);

    input.bindAction("left-fire", MouseButton::Left);
    input.bindAction("left-fire", MouseButton::Middle);

    input.bindAction("right-fire", MouseButton::Right);

    // Make the entity
    auto& entity = engine.make<sill::GameEntity>("camera");
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    auto& cameraComponent = entity.make<sill::CameraComponent>();
    cameraComponent.origin({-2.f, 0.f, 2.f});
    cameraComponent.target({0.f, 0.f, 1.f});
    gameState.camera = &cameraComponent;

    // Behavior for user control
    behaviorComponent.onUpdate([&input, &cameraComponent](float /* dt */) {
        if (input.axisChanged("zoom")) {
            cameraComponent.radiusAdd(-cameraComponent.radius() * input.axis("zoom") / 10.f);
        }

        if (input.axisChanged("main-x") || input.axisChanged("main-y")) {
            glm::vec2 delta(input.axis("main-x") / 100.f, input.axis("main-y") / 100.f);

            // Right click is maintained to translate the camera
            if (input.down("right-fire")) {
                cameraComponent.strafe(delta.x / 10.f, delta.y / 10.f);
            }

            // Left click is maintained to orbit
            if (input.down("left-fire")) {
                cameraComponent.orbitAdd(-delta.x, delta.y);
            }
        }
    });
}
