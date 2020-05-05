#include "./camera.hpp"

#include <lava/crater.hpp>
#include <lava/chamber/math.hpp>

using namespace lava;

void setupCamera(GameState& gameState)
{
    auto& engine = *gameState.engine;
    auto& input = engine.input();

    // Initializing inputs
    input.bindAxis("camera.axis-x", InputAxis::MouseX);
    input.bindAxis("camera.axis-y", InputAxis::MouseY);
    input.bindAxis("camera.zoom", InputAxis::MouseWheelVertical);

    input.bindAction("camera.strafe", MouseButton::Right);
    input.bindAction("camera.orbit", MouseButton::Middle);

    input.bindAction("camera.go-forward", Key::Z);
    input.bindAction("camera.go-backward", Key::S);
    input.bindAction("camera.go-left", Key::Q);
    input.bindAction("camera.go-right", Key::D);
    input.bindAction("camera.go-faster", Key::LeftShift);

    input.bindAction("window.close", Key::Escape);
    input.bindAction("window.toggle-fullscreen", Key::F11);
    input.bindAction("toggle-fps-counting", {Key::LeftControl, Key::LeftAlt, Key::F});

    // Make the entity
    auto& entity = engine.make<sill::GameEntity>("camera");
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    auto& cameraComponent = entity.make<sill::CameraComponent>();
    gameState.camera.component = &cameraComponent;

    // Behavior for user control
    behaviorComponent.onUpdate([&input, &cameraComponent, &gameState](float dt) {
        if (gameState.state == State::Idle && input.justDown("window.close")) {
            gameState.engine->window().close();
        }
        if (input.justDown("window.toggle-fullscreen")) {
            gameState.engine->window().fullscreen(!gameState.engine->window().fullscreen());
        }
        if (input.justDown("toggle-fps-counting")) {
            gameState.engine->fpsCounting(!gameState.engine->fpsCounting());
        }

        // @fixme Better have a "pushLockCamera" function, something callable from anywhere.
        if (gameState.state == State::Editor && gameState.editor.state != EditorState::Idle) return;

        if (gameState.camera.mode == CameraMode::FirstPerson) {
            cameraComponent.go(gameState.player.headPosition);
            cameraComponent.target(gameState.player.headPosition + gameState.player.direction);
        }
        else if (gameState.camera.mode == CameraMode::Orbit) {
            // Going forward/backward
            float forwardImpulse = 0.f;
            if (input.down("camera.go-forward")) forwardImpulse = 3.f; // Meters per second
            else if (input.down("camera.go-backward")) forwardImpulse = -3.f;

            if (std::abs(forwardImpulse) > 0.01f) {
                if (input.down("camera.go-faster")) forwardImpulse *= 3.f;
                cameraComponent.goForward(forwardImpulse * dt, glm::vec3{1, 1, 0});
            }

            // Going right/left
            float rightImpulse = 0.f;
            if (input.down("camera.go-right")) rightImpulse = 2.f;
            else if (input.down("camera.go-left")) rightImpulse = -2.f;

            if (std::abs(rightImpulse) > 0.01f) {
                if (input.down("camera.go-faster")) rightImpulse *= 3.f;
                cameraComponent.goRight(rightImpulse * dt, glm::vec3{1, 1, 0});
            }

            // Mouse control
            if (input.axisChanged("camera.zoom")) {
                cameraComponent.radiusAdd(-cameraComponent.radius() * input.axis("camera.zoom") / 10.f);
            }

            if (input.axisChanged("camera.axis-x") || input.axisChanged("camera.axis-y")) {
                glm::vec2 delta(input.axis("camera.axis-x") / 100.f, input.axis("camera.axis-y") / 100.f);

                if (input.down("camera.strafe")) {
                    cameraComponent.strafe(delta.x / 10.f, delta.y / 10.f);
                }
                if (input.down("camera.orbit")) {
                    cameraComponent.orbitAdd(-delta.x, delta.y);
                }
            }
        }
    });

    // Reticle in first-person camera mode
    auto& reticleEntity = engine.make<sill::GameEntity>("reticle");
    auto& reticleTransformComponent = reticleEntity.make<sill::TransformComponent>();

    auto reticleMaterial = engine.scene().makeMaterial("reticle");
    auto& reticleFlatComponent = reticleEntity.make<sill::FlatComponent>();
    sill::makers::quadFlatMaker({5.f, 5.f})(reticleFlatComponent);
    reticleFlatComponent.primitive(0u, 0u).material(reticleMaterial);

    auto extent = gameState.camera.component->extent();
    reticleTransformComponent.translation2d({extent.width / 2.f, extent.height / 2.f});
    gameState.engine->onWindowExtentChanged([&reticleTransformComponent](const Extent2d& extent) {
        reticleTransformComponent.translation2d({extent.width / 2.f, extent.height / 2.f});
    });

    gameState.camera.reticleEntity = &reticleEntity;

    // Initial setup
    setCameraMode(gameState, (gameState.engine->vr().enabled()) ? CameraMode::Orbit : CameraMode::FirstPerson);
}

void setCameraMode(GameState& gameState, CameraMode mode)
{
    gameState.camera.mode = mode;

    bool firstPersonModeEnabled = (mode == CameraMode::FirstPerson);

    // @todo This reticle thingy update would be useless if we had a dedicated screen-space UI pass.
    gameState.camera.reticleUpdateNeeded = firstPersonModeEnabled;
    gameState.camera.reticleEntity->get<sill::TransformComponent>().scaling2d((firstPersonModeEnabled) ? 1.f : 0.f);

    gameState.engine->window().mouseKeptCentered(mode == CameraMode::FirstPerson);
    gameState.engine->window().mouseHidden(mode == CameraMode::FirstPerson);
}
