#include "./camera.hpp"

#include <iostream>
#include <lava/crater.hpp>
#include <lava/chamber/math.hpp>

using namespace lava;

void setupCamera(GameState& gameState)
{
    auto& engine = *gameState.engine;
    auto& input = engine.input();

    // Initializing inputs
    input.bindAxis("main-x", InputAxis::MouseX);
    input.bindAxis("main-y", InputAxis::MouseY);
    input.bindAxis("zoom", InputAxis::MouseWheelVertical);

    input.bindAction("left-fire", MouseButton::Left);
    input.bindAction("middle-fire", MouseButton::Middle);
    input.bindAction("right-fire", MouseButton::Right);

    input.bindAction("move-forward", Key::Z);
    input.bindAction("move-backward", Key::S);
    input.bindAction("move-left", Key::Q);
    input.bindAction("move-right", Key::D);

    input.bindAction("window.close", Key::Escape);
    input.bindAction("window.toggle-fullscreen", {Key::F11});
    input.bindAction("toggle-fps-counting", {Key::LeftControl, Key::LeftAlt, Key::F});

    // Make the entity
    auto& entity = engine.make<sill::GameEntity>("camera");
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    auto& cameraComponent = entity.make<sill::CameraComponent>();
    cameraComponent.origin({-2.f, 0.f, 1.7f});
    cameraComponent.target({0.f, 0.f, 1.7f});
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

        if (gameState.camera.mode == CameraMode::FirstPerson) {
            if (input.axisChanged("main-x") || input.axisChanged("main-y")) {
                glm::vec2 delta(input.axis("main-x") / 100.f, input.axis("main-y") / 100.f);
                cameraComponent.rotateAtOrigin(-delta.x, -delta.y);
            }
        }
        else if (gameState.camera.mode == CameraMode::Orbit) {
            if (input.axisChanged("zoom")) {
                cameraComponent.radiusAdd(-cameraComponent.radius() * input.axis("zoom") / 10.f);
            }

            if (input.axisChanged("main-x") || input.axisChanged("main-y")) {
                glm::vec2 delta(input.axis("main-x") / 100.f, input.axis("main-y") / 100.f);

                // Right click is maintained to translate the camera
                if (input.down("right-fire")) {
                    cameraComponent.strafe(delta.x / 10.f, delta.y / 10.f);
                }

                // Middle click is maintained to orbit
                if (input.down("middle-fire")) {
                    cameraComponent.orbitAdd(-delta.x, delta.y);
                }
            }
        }
    });

    // Reticle in first-person camera mode
    auto& reticleEntity = engine.make<sill::GameEntity>("reticle");
    auto& reticleTransformComponent = reticleEntity.make<sill::TransformComponent>();

    auto& reticleMaterial = engine.scene().make<magma::Material>("reticle");
    auto& reticleMeshComponent = reticleEntity.make<sill::MeshComponent>();
    sill::makers::planeMeshMaker({1.f, 1.f})(reticleMeshComponent);
    reticleMeshComponent.primitive(0u, 0u).shadowsCastable(false);
    reticleMeshComponent.primitive(0u, 0u).material(reticleMaterial);
    reticleMeshComponent.primitive(0u, 0u).category(RenderCategory::Translucent);

    reticleEntity.make<sill::BehaviorComponent>().onUpdate([&gameState, &reticleTransformComponent](float /* dt */) {
        if (!gameState.camera.reticleUpdateNeeded) return;

        const auto& extent = gameState.camera.component->extent();
        auto coordinates = glm::vec2{0.5f * extent.width, 0.5f * extent.height};
        auto screenMatrix = gameState.camera.component->transformAtCoordinates(coordinates, 0.001f);
        screenMatrix = glm::rotate(screenMatrix, chamber::math::PI_OVER_TWO, {0, 1, 0});
        screenMatrix = glm::scale(screenMatrix, glm::vec3{0.01f});
        reticleTransformComponent.worldTransform(screenMatrix);
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
    if (!firstPersonModeEnabled) {
        gameState.camera.reticleEntity->get<sill::TransformComponent>().scaling(0.f);
    }

    gameState.engine->window().mouseKeptCentered(mode == CameraMode::FirstPerson);
    gameState.engine->window().mouseHidden(mode == CameraMode::FirstPerson);
}
