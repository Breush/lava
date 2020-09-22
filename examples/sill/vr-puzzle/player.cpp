#include "./player.hpp"

#include "./game-state.hpp"
#include "./camera.hpp"
#include "./environment.hpp"

#include <lava/chamber/math.hpp>
#include <glm/gtx/vector_angle.hpp>

using namespace lava;

constexpr const float PLAYER_HEIGHT = 1.75f;

namespace {
    // @note This has been stolen from OrbitCameraController::rotateAtOrigin and might be refactored.
    inline glm::vec3 lookAround(const glm::vec3& direction, const glm::vec2& delta)
    {
        auto axis = glm::vec3(direction.y, -direction.x, 0);

        float latitudeAngle = delta.y;
        auto currentLatitudeAngle = std::asin(direction.z / glm::length(direction));
        if (currentLatitudeAngle + delta.y > chamber::math::PI_OVER_TWO - 0.01) {
            latitudeAngle = chamber::math::PI_OVER_TWO - 0.01 - currentLatitudeAngle;
        }
        else if (currentLatitudeAngle + delta.y < -chamber::math::PI_OVER_TWO + 0.01) {
            latitudeAngle = -chamber::math::PI_OVER_TWO + 0.01 - currentLatitudeAngle;
        }

        auto longitudeDelta = glm::rotateZ(direction, delta.x) - direction;
        auto latitudeDelta = glm::rotate(direction, latitudeAngle, axis) - direction;
        return glm::normalize(direction + longitudeDelta + latitudeDelta);
    }

    void onUpdateMouse(GameState& gameState, float dt)
    {
        if (gameState.state == State::Editor) return;

        auto& input = gameState.engine->input();

        glm::vec3 position = gameState.player.position;
        glm::vec3 direction = gameState.player.direction;

        // Update looking direction
        if (input.axisChanged("player.look-x") || input.axisChanged("player.look-y")) {
            glm::vec2 delta(-input.axis("player.look-x") / 100.f, -input.axis("player.look-y") / 100.f);
            direction = lookAround(direction, delta);
        }

        // Going forward/backward
        float forwardImpulse = 0.f;
        if (input.down("player.move-forward")) forwardImpulse = 3.f; // Meters per second
        else if (input.down("player.move-backward")) forwardImpulse = -3.f;

        if (std::abs(forwardImpulse) > 0.01f) {
            if (input.down("player.run")) forwardImpulse *= 3.f;
            position += forwardImpulse * dt * direction * glm::vec3{1, 1, 0};
        }

        // Going right/left
        float rightImpulse = 0.f;
        if (input.down("player.move-right")) rightImpulse = 2.f;
        else if (input.down("player.move-left")) rightImpulse = -2.f;

        if (std::abs(rightImpulse) > 0.01f) {
            if (input.down("player.run")) rightImpulse *= 3.f;
            auto rightDirection = glm::cross(direction, {0, 0, 1});
            position += rightImpulse * dt * rightDirection * glm::vec3{1, 1, 0};
        }

        // Stick to terrain
        Generic* generic = nullptr;
        Ray ray;
        ray.origin = position + glm::vec3{0.f, 0.f, 20.f};
        ray.direction = glm::vec3{0.f, 0.f, -1.f};
        auto distance = distanceToTerrain(gameState, ray, &generic);
        if (distance != 0.f) {
            position = ray.origin + distance * ray.direction;
        }

        // Storing new info
        gameState.player.direction = direction;

        // Do not walk in water or non-active generics
        if (position.z < -0.2f || (generic != nullptr && !generic->walkable())) {
            return;
        }

        // Do not cross barriers
        if (gameState.player.position != position) {
            for (auto barrier : gameState.level.barriers) {
                if (barrier->teleportationBlocked() &&
                    barrier->intersectSegment(gameState.player.position, position)) {
                    return;
                }
            }

            gameState.player.position = position;
        }

        gameState.player.headPosition = gameState.player.position;
        gameState.player.headPosition.z += PLAYER_HEIGHT;
    }

    void onUpdateVr(GameState& gameState)
    {
        // When the player is moved programatically (e.g. when loading a new level).
        if (gameState.player.vrAreaPosition != gameState.engine->vr().translation()) {
            gameState.engine->vr().translation(gameState.player.vrAreaPosition);
        }

        // @note This is always in sync, and no direction to update.
        // gameState.player.vrAreaPosition = gameState.engine->vr().translation();

        gameState.player.headPosition = gameState.engine->vr().deviceTransform(VrDeviceType::Head).translation;

        // If any hand of the player is crossing a barrier, vibrate.
        const auto& leftHandTranslation = gameState.engine->vr().deviceTransform(VrDeviceType::LeftHand).translation;
        const auto& rightHandTranslation = gameState.engine->vr().deviceTransform(VrDeviceType::RightHand).translation;
        bool invalidLeftHand = !gameState.engine->vr().deviceValid(VrDeviceType::LeftHand);
        bool invalidRightHand = !gameState.engine->vr().deviceValid(VrDeviceType::RightHand);
        for (auto barrier : gameState.level.barriers) {
            if (barrier->teleportationBlocked()) {
                if (!invalidLeftHand && barrier->intersectSegment(gameState.player.position, leftHandTranslation)) {
                    invalidLeftHand = true;
                }
                if (!invalidRightHand && barrier->intersectSegment(gameState.player.position, rightHandTranslation)) {
                    invalidRightHand = true;
                }
            }

            if (invalidLeftHand && invalidRightHand) {
                break;
            }
        }

        gameState.engine->vr().deviceVibrationEnabled(VrDeviceType::LeftHand, invalidLeftHand);
        gameState.engine->vr().deviceVibrationEnabled(VrDeviceType::RightHand, invalidRightHand);
    }
}

void setupPlayer(GameState& gameState)
{
    auto& engine = *gameState.engine;
    auto& input = engine.input();

    // Mouse
    input.bindAxis("player.look-x", InputAxis::MouseX);
    input.bindAxis("player.look-y", InputAxis::MouseY);

    input.bindAction("player.grab-brick", MouseButton::Left);
    input.bindAction("player.rotate-brick", MouseButton::Right);

    input.bindAction("player.move-forward", Key::Z);
    input.bindAction("player.move-backward", Key::S);
    input.bindAction("player.move-left", Key::Q);
    input.bindAction("player.move-right", Key::D);
    input.bindAction("player.run", Key::LeftShift);

    // VR
    if (engine.vr().enabled()) {
        engine.input().bindAction("player.grab-brick", VrButton::Trigger, VrDeviceType::RightHand);
        engine.input().bindAction("player.teleport", VrButton::Touchpad, VrDeviceType::RightHand);
    }

    auto& entity = engine.make<sill::Entity>("player");
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&](float dt) {
        if (engine.vr().enabled()) {
            onUpdateVr(gameState);
        }
        else {
            onUpdateMouse(gameState, dt);
        }
    });
}
