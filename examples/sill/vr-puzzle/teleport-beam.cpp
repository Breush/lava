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
            gameState.teleportBeamEntity->get<sill::TransformComponent>().scaling({1, 1, 1});
            rayPickingEnabled(gameState, false);
        }

        if (gameState.state != State::TeleportBeam) return;

        if (engine.input().justUp("touchpad")) {
            gameState.state = State::Idle;
            gameState.teleportBeamEntity->get<sill::TransformComponent>().scaling({0, 0, 0});
            gameState.teleportAreaEntity->get<sill::TransformComponent>().scaling({0, 0, 0});
            rayPickingEnabled(gameState, true);
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

        gameState.teleportBeamEntity->get<sill::TransformComponent>().worldTransform(
            glm::rotate(glm::translate(glm::mat4(1.f), translation), angles.z, {0, 0, 1}));

        auto& teleportBeamPrimitive = gameState.teleportBeamEntity->get<sill::MeshComponent>().primitive(0, 0);

        // @fixme Have state info to prevent useless updates
        // ---> Can this be a mechanic inside material() ?
        bool invalid = (angles.x < math::PI * 0.15f) || (angles.x > math::PI * 0.75f);
        teleportBeamPrimitive.material()->set("invalid", invalid);

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
            if (invalid && i > 2) break;

            // @note We make more points at the start
            const float t = (0.05f * i) * (0.05f * i);
            const auto delta = velocity * t + 0.5f * acceleration * t * t;

            positions[i] = originLeft + delta;
            positions[i + 32u] = originRight + delta;
        }
        teleportBeamPrimitive.verticesPositions(positions);

        if (invalid) {
            gameState.teleportAreaEntity->get<sill::TransformComponent>().scaling({0, 0, 0});
            return;
        }
        else {
            gameState.teleportAreaEntity->get<sill::TransformComponent>().scaling({1, 1, 1});
        }

        // Detect where the beam lands!
        // @note For f(t) = f(0) + v*t + a*t*t/2,
        // we find t0 such that f(t0).z = 0,
        // and thus we have f(t0) as the point of interest
        float t0 = (-velocity.z - std::sqrt(velocity.z * velocity.z - 2 * acceleration.z * translation.z)) / acceleration.z;

        glm::vec3 target = translation + velocity * t0 + 0.5f * acceleration * t0 * t0;
        target = translation + glm::rotate(target - translation, angles.z, {0.f, 0.f, 1.f});
        target.z += 0.125f; // Forcing it to show full cylinder (length / 2).

        gameState.teleportAreaEntity->get<sill::TransformComponent>().translation(target);

        if (engine.input().justDown("trigger")) {
            // We want to teleport ourselves, so we say that we want our head to be
            // at the target point after teleportation.
            auto headTranslation = glm::vec3(engine.vr().deviceTransform(VrDeviceType::Head)[3]) - engine.vr().translation();
            target.x -= headTranslation.x;
            target.y -= headTranslation.y;
            target.z = 0.f;

            engine.vr().translation(target);
        }
    }
}

void setupTeleportBeam(GameState& gameState)
{
    auto& engine = *gameState.engine;
    if (!gameState.engine->vr().enabled()) return;

    // Beam
    auto& teleportBeamEntity = engine.make<sill::GameEntity>("teleport-beam");
    auto& meshComponent = teleportBeamEntity.make<sill::MeshComponent>();
    gameState.teleportBeamEntity = &teleportBeamEntity;

    sill::makers::PlaneMeshOptions planeMeshOptions;
    planeMeshOptions.tessellation = {2u, 32u};
    planeMeshOptions.doubleSided = true;
    sill::makers::planeMeshMaker({1.f, 1.f}, planeMeshOptions)(meshComponent);

    teleportBeamEntity.get<sill::TransformComponent>().scaling({0, 0, 0});
    auto& teleportBeamMaterial = engine.scene().make<magma::Material>("teleport-beam");
    teleportBeamMaterial.set("length", 32.f);
    meshComponent.primitive(0, 0).material(teleportBeamMaterial);
    meshComponent.primitive(0, 0).category(RenderCategory::Translucent);
    meshComponent.primitive(0, 0).shadowsCastable(false);

    // Area
    auto& teleportAreaEntity = engine.make<sill::GameEntity>("teleport-area");
    auto& teleportAreaMeshComponent = teleportAreaEntity.make<sill::MeshComponent>();
    gameState.teleportAreaEntity = &teleportAreaEntity;

    teleportAreaEntity.get<sill::TransformComponent>().scaling({0, 0, 0});
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
