#include "./teleport-beam.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <lava/chamber/math.hpp>

#include "./ray-picking.hpp"

using namespace lava;
using namespace lava::chamber;

namespace {
    void onUpdateVr(GameState& gameState)
    {
        auto& engine = *gameState.engine;
        if (!engine.vr().deviceValid(VrDeviceType::RightHand)) return;

        if (gameState.state == State::Idle && engine.input().justDown("touchpad")) {
            gameState.state = State::TeleportBeam;
            gameState.teleport.beamEntity->get<sill::TransformComponent>().scaling(1.f);
            rayPickingEnabled(gameState, false);
        }

        if (gameState.state != State::TeleportBeam) return;

        if (engine.input().justUp("touchpad")) {
            gameState.state = State::Idle;
            gameState.teleport.beamEntity->get<sill::TransformComponent>().scaling(0.f);
            gameState.teleport.areaEntity->get<sill::TransformComponent>().scaling(0.f);
            rayPickingEnabled(gameState, true);

            // We want to teleport ourselves, so we say that we want our head to be
            // at the target point after teleportation.
            if (gameState.teleport.valid) {
                auto target = gameState.teleport.target;
                auto headTranslation = glm::vec3(engine.vr().deviceTransform(VrDeviceType::Head)[3]) - engine.vr().translation();
                target.x -= headTranslation.x;
                target.y -= headTranslation.y;
                target.z = 0.f;
                engine.vr().translation(target);
            }
            return;
        }

        // Extract hand components
        glm::vec3 scale;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::quat orientation;
        auto handTransform = engine.vr().deviceTransform(VrDeviceType::RightHand);
        glm::decompose(handTransform, scale, orientation, translation, skew, perspective);
        glm::vec3 angles = glm::eulerAngles(orientation);

        gameState.teleport.beamEntity->get<sill::TransformComponent>().worldTransform(
            glm::rotate(glm::translate(glm::mat4(1.f), translation), angles.z, {0, 0, 1}));

        auto& teleportBeamPrimitive = gameState.teleport.beamEntity->get<sill::MeshComponent>().primitive(0, 0);
        bool anglesValid = !(angles.x < math::PI * 0.15f) && !(angles.x > math::PI * 0.75f);
        teleportBeamPrimitive.material()->set("anglesValid", anglesValid);

        constexpr const float velocityPower = 8.f;
        const float velocityFactor = (angles.x - math::PI_OVER_TWO) / math::PI_OVER_FOUR;

        const glm::vec3 velocity = {0.f, velocityPower, velocityPower * velocityFactor};
        const glm::vec3 acceleration = {0.f, 0.f, -9.81f};

        constexpr const float beamWidth = 0.02f;
        glm::vec3 originLeft = {-beamWidth / 2.f, 0.f, 0.f};
        glm::vec3 originRight = {beamWidth / 2.f, 0.f, 0.f};

        // Dynamically update the teleport beam.
        static std::vector<glm::vec3> positions(teleportBeamPrimitive.verticesCount());
        for (auto i = 0u; i < 32u; ++i) {
            if (!anglesValid && i > 2) break;

            // @note We make more points at the start
            const float t = (0.05f * i) * (0.05f * i);
            const auto delta = velocity * t + 0.5f * acceleration * t * t;

            positions[i] = originLeft + delta;
            positions[i + 32u] = originRight + delta;
        }
        teleportBeamPrimitive.verticesPositions(positions);

        // Detect where the beam lands!
        // @note For f(t) = f(0) + v*t + a*t*t/2,
        // we find t0 such that f(t0).z = 0,
        // and thus we have f(t0) as the point of interest
        float t0 = (-velocity.z - std::sqrt(velocity.z * velocity.z - 2 * acceleration.z * translation.z)) / acceleration.z;

        glm::vec3 target = translation + velocity * t0 + 0.5f * acceleration * t0 * t0;
        target = translation + glm::rotate(target - translation, angles.z, {0.f, 0.f, 1.f});
        target.z += 0.125f; // Forcing it to show full cylinder (length / 2).

        gameState.teleport.areaEntity->get<sill::TransformComponent>().translation(target);
        gameState.teleport.target = target;

        // Teleportation is valid if:
        // - all angles are not to sharp
        // - no barrier prevents us to do so
        bool placeValid = true;
        if (anglesValid) {
            for (const auto& barrier : gameState.level.barriers) {
                if (barrier->powered()) continue;

                auto barrierPosition = glm::vec2(barrier->transform().translation());
                if (glm::distance(glm::vec2(target), barrierPosition) < barrier->diameter() / 2.f) {
                    placeValid = false;
                    break;
                }
            }
        }
        teleportBeamPrimitive.material()->set("placeValid", placeValid);

        // Hiding the target area entity when invalid
        gameState.teleport.valid = anglesValid && placeValid;
        gameState.teleport.areaEntity->get<sill::TransformComponent>().scaling((gameState.teleport.valid) ? 1.f : 0.f);
    }
}

void setupTeleportBeam(GameState& gameState)
{
    auto& engine = *gameState.engine;
    if (!gameState.engine->vr().enabled()) return;

    // Beam
    auto& teleportBeamEntity = engine.make<sill::GameEntity>("teleport-beam");
    auto& meshComponent = teleportBeamEntity.make<sill::MeshComponent>();
    gameState.teleport.beamEntity = &teleportBeamEntity;

    sill::makers::PlaneMeshOptions planeMeshOptions;
    planeMeshOptions.tessellation = {2u, 32u};
    planeMeshOptions.doubleSided = true;
    sill::makers::planeMeshMaker({1.f, 1.f}, planeMeshOptions)(meshComponent);

    teleportBeamEntity.get<sill::TransformComponent>().scaling(0.f);
    auto& teleportBeamMaterial = engine.scene().make<magma::Material>("teleport-beam");
    teleportBeamMaterial.set("length", 32.f);
    meshComponent.primitive(0, 0).material(teleportBeamMaterial);
    meshComponent.primitive(0, 0).category(RenderCategory::Translucent);
    meshComponent.primitive(0, 0).shadowsCastable(false);

    // Area
    auto& teleportAreaEntity = engine.make<sill::GameEntity>("teleport-area");
    auto& teleportAreaMeshComponent = teleportAreaEntity.make<sill::MeshComponent>();
    gameState.teleport.areaEntity = &teleportAreaEntity;

    teleportAreaEntity.get<sill::TransformComponent>().scaling(0.f);
    auto& teleportAreaMaterial = engine.scene().make<magma::Material>("teleport-area");
    sill::makers::cylinderMeshMaker(32u, 0.75f, 0.25f, {.doubleSided = true})(teleportAreaMeshComponent);
    teleportAreaMeshComponent.primitive(0, 0).material(teleportAreaMaterial);
    teleportAreaMeshComponent.primitive(0, 0).category(RenderCategory::Translucent);
    teleportAreaMeshComponent.primitive(0, 0).shadowsCastable(false);

    // Update
    auto& behaviorComponent = teleportBeamEntity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&](float dt) {
        static float time = 0.f;
        time += dt;
        teleportBeamMaterial.set("time", time);

        onUpdateVr(gameState);
    });
}
