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
                engine.vr().translation(target);
            }
            return;
        }

        // Extract hand components
        glm::vec3 scale, translation, skew;
        glm::vec4 perspective;
        glm::quat orientation;
        auto handTransform = engine.vr().deviceTransform(VrDeviceType::RightHand);
        glm::decompose(handTransform, scale, orientation, translation, skew, perspective);
        glm::vec3 angles = glm::eulerAngles(orientation);

        const auto beamWorldTransform = glm::rotate(glm::translate(glm::mat4(1.f), translation), angles.z, {0, 0, 1});
        gameState.teleport.beamEntity->get<sill::TransformComponent>().worldTransform(beamWorldTransform);

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
        bool targetFound = false;
        glm::vec3 previousDelta;
        glm::vec3 target;
        for (auto i = 0u; i < 32u; ++i) {
            if (!anglesValid && i > 2) break;

            // @note We make more points at the start
            const float t = (0.05f * i) * (0.05f * i);
            glm::vec3 delta = velocity * t + 0.5f * acceleration * t * t;

            positions[i] = originLeft + delta;
            positions[i + 32u] = originRight + delta;

            // Detect where the beam lands!
            if (!targetFound && i > 0u) {
                // Go to world-space
                Ray ray;
                ray.origin = (positions[i - 1u] + positions[(i + 32u) - 1u]) / 2.f;
                ray.origin = beamWorldTransform * glm::vec4(ray.origin, 1.f);

                // @note We just don't check if the ray seems off-world.
                if (ray.origin.z > -20.f) {
                    ray.direction = glm::normalize(delta - previousDelta);
                    ray.direction = beamWorldTransform * glm::vec4(ray.direction, 0.f);

                    auto distance = gameState.terrain.entity->distanceFrom(ray, sill::PickPrecision::Collider);
                    if (distance != 0.f && distance <= glm::length(delta - previousDelta)) {
                        target = ray.origin + distance * ray.direction;
                        targetFound = true;
                    }
                }
            }

            previousDelta = delta;
        }
        teleportBeamPrimitive.verticesPositions(positions);

        if (targetFound) {
            // @note Offset is to show full cylinder (length / 2).
            gameState.teleport.areaEntity->get<sill::TransformComponent>().translation(target + glm::vec3{0.f, 0.f, 0.125f});
            gameState.teleport.target = target;
        }

        // Teleportation is valid if:
        // - the terrain has been hit (targetFound)
        // - all angles are not to sharp
        // - no barrier prevents us to do so
        bool placeValid = targetFound;
        if (targetFound && anglesValid) {
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
    auto teleportBeamMaterial = engine.scene().makeMaterial("teleport-beam");
    teleportBeamMaterial->set("length", 32.f);
    meshComponent.primitive(0, 0).material(teleportBeamMaterial);
    meshComponent.primitive(0, 0).category(RenderCategory::Translucent);
    meshComponent.primitive(0, 0).shadowsCastable(false);

    // Area
    auto& teleportAreaEntity = engine.make<sill::GameEntity>("teleport-area");
    auto& teleportAreaMeshComponent = teleportAreaEntity.make<sill::MeshComponent>();
    gameState.teleport.areaEntity = &teleportAreaEntity;

    teleportAreaEntity.get<sill::TransformComponent>().scaling(0.f);
    auto teleportAreaMaterial = engine.scene().makeMaterial("teleport-area");
    sill::makers::cylinderMeshMaker(32u, 0.75f, 0.25f, {.doubleSided = true})(teleportAreaMeshComponent);
    teleportAreaMeshComponent.primitive(0, 0).material(teleportAreaMaterial);
    teleportAreaMeshComponent.primitive(0, 0).category(RenderCategory::Translucent);
    teleportAreaMeshComponent.primitive(0, 0).shadowsCastable(false);

    // Update
    auto& behaviorComponent = teleportBeamEntity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&gameState, material = teleportBeamMaterial.get()](float dt) {
        static float time = 0.f;
        time += dt;
        material->set("time", time);

        onUpdateVr(gameState);
    });
}
