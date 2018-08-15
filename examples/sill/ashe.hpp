#pragma once

#include <lava/sill.hpp>

#include <iostream>

namespace lava::ashe {
    class Application {
    public:
        Application()
        {
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

            auto& entity = m_engine.make<sill::GameEntity>();

            auto& cameraComponent = entity.make<sill::CameraComponent>();
            cameraComponent.translation({-2.f, 3.f, 2.f});
            cameraComponent.target({0.f, 0.f, 0.f});

            auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
            behaviorComponent.onUpdate([&input, &cameraComponent]() {
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
        }

        sill::GameEngine& engine() { return m_engine; }

    private:
        sill::GameEngine m_engine;
    };
}
