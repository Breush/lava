#pragma once

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

        /// Whether the specified action has gone from up to down in last update block.
        bool justDown(const std::string& actionName) const;

        /**
         * @name Binding
         */
        /// @{
        void bindAction(const std::string& actionName, MouseButton mouseButton);
        void bindAction(const std::string& actionName, Key key);

        // @todo Axes
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
        /// @}

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
