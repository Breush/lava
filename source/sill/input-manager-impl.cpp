#include "./input-manager-impl.hpp"

using namespace lava::sill;

bool InputManager::Impl::down(const std::string& actionName) const
{
    const auto& action = m_actions.at(actionName);
    return action.activeness > 0;
}

bool InputManager::Impl::justDown(const std::string& actionName) const
{
    const auto& action = m_actions.at(actionName);
    return action.activeness > 0 && action.previousActiveness == 0;
}

void InputManager::Impl::bindAction(const std::string& actionName, MouseButton mouseButton)
{
    m_actions[actionName].mouseButtons.emplace(mouseButton);
}

void InputManager::Impl::bindAction(const std::string& actionName, Key key)
{
    m_actions[actionName].keys.emplace(key);
}

void InputManager::Impl::updateReset()
{
    for (auto& iAction : m_actions) {
        auto& action = iAction.second;
        action.previousActiveness = action.activeness;
    }
}

void InputManager::Impl::update(WsEvent& event)
{
    // Mouse buttons
    if (event.type == WsEventType::MouseButtonPressed) {
        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            if (action.mouseButtons.find(event.mouseButton.which) != action.mouseButtons.end()) {
                action.activeness += 1u;
            }
        }
    }
    else if (event.type == WsEventType::MouseButtonReleased) {
        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            if (action.mouseButtons.find(event.mouseButton.which) != action.mouseButtons.end()) {
                action.activeness -= 1u;
            }
        }
    }

    // Keys
    else if (event.type == WsEventType::KeyPressed) {
        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            if (action.keys.find(event.key.which) != action.keys.end()) {
                action.activeness += 1u;
            }
        }
    }
    else if (event.type == WsEventType::KeyReleased) {
        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            if (action.keys.find(event.key.which) != action.keys.end()) {
                action.activeness -= 1u;
            }
        }
    }
}
