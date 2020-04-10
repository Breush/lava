#pragma once

#include <lava/core/input-axis.hpp>
#include <lava/core/vr-event.hpp>
#include <lava/core/ws-event.hpp>
#include <set>
#include <string>

namespace lava::sill {
    /// Handles raw hardware input and convert them to user-defined actions or axes.
    class InputManager {
    public:
        /// Whether the specified action is held down.
        bool down(const std::string& actionName) const;

        /// Whether the specified action is left up.
        bool up(const std::string& actionName) const;

        /// Whether the specified action has gone from up to down in last update block.
        bool justDown(const std::string& actionName) const;

        /// Whether the specified action has gone from down to up in last update block.
        bool justUp(const std::string& actionName) const;

        /// Whether the specified action has gone from up to down in last update block,
        /// and the action occured without any other change (i.e. no other input from user).
        /// This is basically a one-time key/button push without holding.
        bool justDownUp(const std::string& actionName) const;

        /// Whether the specified axis has changed.
        bool axisChanged(const std::string& axisName) const;

        /// Get the value of a registered axis.
        float axis(const std::string& axisName) const;

        /// Mouse position in window, from (0, 0) top-left to (windowWidth, windowHeight) bottom-right.
        const glm::vec2& mouseCoordinates() const { return m_mouseCoordinates; }

        /**
         * @name Binding
         */
        /// @{
        void bindAction(const std::string& actionName, MouseButton mouseButton);
        void bindAction(const std::string& actionName, VrButton vrButton, VrDeviceType hand);
        void bindAction(const std::string& actionName, Key key);
        void bindAction(const std::string& actionName, const std::set<Key>& keys);

        void bindAxis(const std::string& axisName, InputAxis inputAxis);
        /// @}

        /**
         * @name Update
         * Used by the game engine mainly.
         */
        /// @{
        /**
         * Start an update block.
         * All following updates will be seen as grouped,
         * and previous downing actions will be updated accordingly.
         */
        void updateReset();

        /// Update the internal state according to the windowing system event.
        void update(WsEvent& event);

        /// Update the internal state according to the virtual reality event.
        void update(VrEvent& event);
        /// @}

    protected:
        // Check whether all keys are pressed.
        bool keysPressed(const std::set<Key>& keys) const;

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
            std::vector<std::set<Key>> keys; //!< Vector represents OR, set represents AND.
        };

        struct Axis {
            float value = 0.f; //!< Current value of the axis.
            std::set<InputAxis> inputAxes;
        };

    private:
        std::unordered_map<std::string, Action> m_actions;
        std::unordered_map<std::string, Axis> m_axes;
        std::unordered_map<Key, bool> m_keysPressed;
        std::set<std::string> m_previousUpdatedBindings;
        std::set<std::string> m_updatedBindings;

        glm::vec2 m_mouseCoordinates; // Last known mouse position.
    };
}
