#pragma once

#include <lava/sill/input-manager.hpp>

#include <set>
#include <unordered_map>

namespace lava::sill {
    class InputManager::Impl {
    public:
        // InputManager
        bool justDown(const std::string& actionName) const;

        void bindAction(const std::string& actionName, crater::input::Button button);
        void bindAction(const std::string& actionName, crater::input::Key key);

        void updateReset();
        void update(crater::Event& event);

    private:
        struct Action {
            uint8_t activeness = 0u;         //!< Number of keys or buttons down.
            uint8_t previousActiveness = 0u; //!< Number of keys or buttons down in the previous update block.
            std::set<crater::input::Button> mouseButtons;
            std::set<crater::input::Key> keys;
        };

    private:
        std::unordered_map<std::string, Action> m_actions;
    };
}
