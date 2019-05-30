#pragma once

#include <lava/sill.hpp>

namespace lava::ashe {
    class Application {
    public:
        Application()
        {
            //----- Initializing materials

            m_engine.registerMaterialFromFile("skybox", "./data/shaders/materials/skybox-material.shmag");
            m_engine.registerMaterialFromFile("matcap", "./data/shaders/materials/matcap-material.shmag");

            //----- Initializing inputs

            auto& input = m_engine.input();

            input.bindAxis("main-x", InputAxis::MouseX);
            input.bindAxis("main-y", InputAxis::MouseY);
            input.bindAxis("zoom", InputAxis::MouseWheelVertical);

            input.bindAction("left-fire", MouseButton::Left);

            input.bindAction("right-fire", MouseButton::Right);
            input.bindAction("right-fire", Key::LeftAlt);
            input.bindAction("right-fire", Key::RightAlt);

            //----- Initializing camera

            auto& cameraEntity = m_engine.make<sill::GameEntity>();
            auto& cameraComponent = cameraEntity.make<sill::CameraComponent>();
            cameraComponent.translation({-2.f, 3.f, 2.f});
            cameraComponent.target({0.f, 0.f, 0.f});

            auto& cameraBehaviorComponent = cameraEntity.make<sill::BehaviorComponent>();
            cameraBehaviorComponent.onUpdate([&input, &cameraComponent](float /* dt */) {
                if (input.axisChanged("zoom")) {
                    cameraComponent.radiusAdd(-cameraComponent.radius() * input.axis("zoom") / 10.f);
                }

                if (input.axisChanged("main-x") || input.axisChanged("main-y")) {
                    // @fixme These factors are magic.
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

            //----- Skybox

            m_engine.environmentTexture("./assets/skies/cloudy/");

            // @fixme We could draw the sky into another viewport,
            // and compose the final image.
            // Or have it in some pre-pass group with depth disabled.
            auto& skyboxEntity = m_engine.make<sill::GameEntity>();
            auto& skyboxTransformComponent = skyboxEntity.ensure<sill::TransformComponent>();

            auto& skyMeshComponent = skyboxEntity.make<sill::MeshComponent>();
            sill::makers::BoxMeshOptions options;
            options.siding = sill::BoxSiding::In;
            options.coordinatesSystem = sill::BoxCoordinatesSystem::Box2x3;
            sill::makers::boxMeshMaker(80.f, options)(skyMeshComponent);

            auto& skyboxMaterial = m_engine.make<sill::Material>("skybox");
            skyboxMaterial.set("useEnvironmentMap", true);
            skyboxMaterial.set("lod", 1u);
            skyMeshComponent.node(0).mesh->primitive(0).material(skyboxMaterial);

            // We keep the sky centered in the camera
            // @fixme The CameraComponent does not care about TransformComponent,
            // meaning it does not send onWorldTransformChanged.
            auto& skyboxBehaviorComponent = skyboxEntity.make<sill::BehaviorComponent>();
            skyboxBehaviorComponent.onUpdate(
                [&](float /* dt */) { skyboxTransformComponent.translation(cameraComponent.translation()); });
        }

        sill::GameEngine& engine() { return m_engine; }

    private:
        sill::GameEngine m_engine;
    };
}
