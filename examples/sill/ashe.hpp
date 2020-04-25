#pragma once

#include <lava/sill.hpp>
#include <lava/magma.hpp>
#include <lava/crater.hpp>

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

            input.bindAction("middle-fire", MouseButton::Middle);
            input.bindAction("middle-fire", MouseButton::Left);

            input.bindAction("right-fire", MouseButton::Right);
            input.bindAction("right-fire", Key::LeftAlt);
            input.bindAction("right-fire", Key::RightAlt);

            input.bindAction("window.close", Key::Escape);
            input.bindAction("window.toggle-fullscreen", {Key::F11});
            input.bindAction("toggle-fps-counting", {Key::LeftControl, Key::LeftAlt, Key::F});
            input.bindAction("toggle-msaa", {Key::LeftControl, Key::LeftAlt, Key::M});

            //----- Initializing camera

            auto& cameraEntity = m_engine.make<sill::GameEntity>("ashe.camera");
            auto& cameraComponent = cameraEntity.make<sill::CameraComponent>();
            cameraComponent.origin({-2.f, -3.f, 2.f});
            cameraComponent.target({0.f, 0.f, 0.f});

            auto& cameraBehaviorComponent = cameraEntity.make<sill::BehaviorComponent>();
            cameraBehaviorComponent.onUpdate([this, &input, &cameraComponent](float /* dt */) {
                if (input.justDown("window.close")) {
                    m_engine.window().close();
                }
                if (input.justDown("window.toggle-fullscreen")) {
                    m_engine.window().fullscreen(!m_engine.window().fullscreen());
                }
                if (input.justDown("toggle-fps-counting")) {
                    m_engine.fpsCounting(!m_engine.fpsCounting());
                }
                if (input.justDown("toggle-msaa")) {
                    m_engine.scene().msaa((m_engine.scene().msaa() == magma::Msaa::None) ? magma::Msaa::Max : magma::Msaa::None);
                }

                if (input.axisChanged("zoom")) {
                    cameraComponent.radiusAdd(-cameraComponent.radius() * input.axis("zoom") / 10.f);
                }

                if (input.axisChanged("main-x") || input.axisChanged("main-y")) {
                    // @todo These factors are magic?
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
            });

            //----- Skybox

            m_engine.environmentTexture("./assets/skies/cloudy/");

            auto& skyboxEntity = m_engine.make<sill::GameEntity>("ashe.skybox");
            auto& skyMeshComponent = skyboxEntity.make<sill::MeshComponent>();
            sill::makers::BoxMeshOptions options{.siding = sill::BoxSiding::In};
            sill::makers::boxMeshMaker(1.f, options)(skyMeshComponent);
            skyMeshComponent.category(RenderCategory::Depthless);

            auto skyboxMaterial = m_engine.scene().makeMaterial("skybox");
            skyboxMaterial->set("useEnvironmentMap", true);
            skyboxMaterial->set("lod", 1u);
            skyMeshComponent.primitive(0, 0).material(skyboxMaterial);
        }

        sill::GameEngine& engine() { return m_engine; }

    private:
        sill::GameEngine m_engine;
    };
}
