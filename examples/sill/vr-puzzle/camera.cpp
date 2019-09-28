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

    input.bindAction("move-forward", Key::Z);
    input.bindAction("move-backward", Key::S);
    input.bindAction("move-left", Key::Q);
    input.bindAction("move-right", Key::D);

    // Make the entity
    auto& entity = engine.make<sill::GameEntity>("camera");
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    auto& cameraComponent = entity.make<sill::CameraComponent>();
    cameraComponent.origin({-2.f, 0.f, 1.7f});
    cameraComponent.target({0.f, 0.f, 1.7f});
    gameState.camera = &cameraComponent;

    // Behavior for user control
    behaviorComponent.onUpdate([&input, &cameraComponent, &gameState](float dt) {
        // @fixme Better have a "pushLockCamera" function, something callable from anywhere.
        if (gameState.state == State::Editor && gameState.editor.state != EditorState::Idle) return;

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

        // Going forward/backward

        static float forwardImpulse = 0.f;

        if (input.down("move-forward")) {
            forwardImpulse = 3.f; // meters per second
        }
        else if (input.down("move-backward")) {
            forwardImpulse = -3.f;
        }

        if (std::abs(forwardImpulse) > 0.01f) {
            cameraComponent.goForward(forwardImpulse * dt, {1, 1, 0});
            forwardImpulse = 0.9f * forwardImpulse;
        }

        // Going right/left

        static float rightImpulse = 0.f;

        if (input.down("move-right")) {
            rightImpulse = 2.f;
        }
        else if (input.down("move-left")) {
            rightImpulse = -2.f;
        }

        if (std::abs(rightImpulse) > 0.01f) {
            cameraComponent.goRight(rightImpulse * dt, {1, 1, 0});
            rightImpulse = 0.9f * rightImpulse;
        }
    });
}
