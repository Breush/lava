#pragma once

#include <lava/sill/input-manager.hpp>

namespace lava::sill {
    class InputManager::Impl {
    public:
        // InputManager
        bool down(const std::string& actionName) const;
        bool up(const std::string& actionName) const;
        bool justDown(const std::string& actionName) const;
        bool justUp(const std::string& actionName) const;
        bool axisChanged(const std::string& axisName) const;
        float axis(const std::string& axisName) const;
        const glm::vec2& mouseCoordinates() const { return m_mouseCoordinates; }

        void bindAction(const std::string& actionName, MouseButton mouseButton);
        void bindAction(const std::string& actionName, VrButton vrButton, VrDeviceType hand);
        void bindAction(const std::string& actionName, Key key);
        void bindAxis(const std::string& axisName, InputAxis inputAxis);

        void updateReset();
        void update(WsEvent& event);
        void update(VrEvent& event);

    private:
        struct VrControllerButton {
            VrDeviceType hand; // Which controller (left, right or any).
            VrButton button;   // Which button is pressed or released.
        };

        struct Action {
            uint8_t activeness = 0u;         //!< Number of keys or buttons down.
            uint8_t previousActiveness = 0u; //!< Number of keys or buttons down in the previous update block.
            std::set<MouseButton> mouseButtons;
            std::vector<VrControllerButton> vrControllerButtons;
            std::set<Key> keys;
        };

        struct Axis {
            float value = 0.f; //!< Current value of the axis.
            std::set<InputAxis> inputAxes;
        };

    private:
        std::unordered_map<std::string, Action> m_actions;
        std::unordered_map<std::string, Axis> m_axes;

        glm::vec2 m_mouseCoordinates; // Last known mouse position.
        bool m_initializingMousePosition = true;
    };
}
