#pragma once

#include <lava/core/input-axis.hpp>
#include <lava/core/vr-event.hpp>
#include <lava/core/ws-event.hpp>
#include <string>

namespace lava::sill {
    /// Handles raw hardware input and convert them to user-defined actions or axes.
    class InputManager {
    public:
        InputManager();
        ~InputManager();

        /// Whether the specified action is held down.
        bool down(const std::string& actionName) const;

        /// Whether the specified action is left up.
        bool up(const std::string& actionName) const;

        /// Whether the specified action has gone from up to down in last update block.
        bool justDown(const std::string& actionName) const;

        /// Whether the specified action has gone from down to up in last update block.
        bool justUp(const std::string& actionName) const;

        /// Whether the specified axis has changed.
        bool axisChanged(const std::string& axisName) const;

        /// Get the value of a registered axis.
        float axis(const std::string& axisName) const;

        /// Mouse position in window, from (0, 0) top-left to (windowWidth, windowHeight) bottom-right.
        const glm::vec2& mouseCoordinates() const;

        /**
         * @name Binding
         */
        /// @{
        void bindAction(const std::string& actionName, MouseButton mouseButton);
        void bindAction(const std::string& actionName, VrButton vrButton, VrDeviceType hand);
        void bindAction(const std::string& actionName, Key key);

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

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
