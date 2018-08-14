#pragma once

#include <lava/sill/input-manager.hpp>

#include <glm/vec2.hpp>
#include <set>
#include <unordered_map>

namespace lava::sill {
    class InputManager::Impl {
    public:
        // InputManager
        bool down(const std::string& actionName) const;
        bool justDown(const std::string& actionName) const;
        bool axisChanged(const std::string& axisName) const;
        float axis(const std::string& axisName) const;

        void bindAction(const std::string& actionName, MouseButton mouseButton);
        void bindAction(const std::string& actionName, Key key);
        void bindAxis(const std::string& axisName, InputAxis inputAxis);

        void updateReset();
        void update(WsEvent& event);

    private:
        struct Action {
            uint8_t activeness = 0u;         //!< Number of keys or buttons down.
            uint8_t previousActiveness = 0u; //!< Number of keys or buttons down in the previous update block.
            std::set<MouseButton> mouseButtons;
            std::set<Key> keys;
        };

        struct Axis {
            float value = 0.f; //!< Current value of the axis.
            std::set<InputAxis> inputAxes;
        };

    private:
        std::unordered_map<std::string, Action> m_actions;
        std::unordered_map<std::string, Axis> m_axes;

        glm::vec2 m_mousePosition; // Last known mouse position.
        bool m_initializingMousePosition = true;
    };
}
