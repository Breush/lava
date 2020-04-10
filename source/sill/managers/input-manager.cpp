#include <lava/sill/managers/input-manager.hpp>

using namespace lava::sill;

bool InputManager::down(const std::string& actionName) const
{
    const auto& action = m_actions.at(actionName);
    return action.activeness > 0;
}

bool InputManager::up(const std::string& actionName) const
{
    const auto& action = m_actions.at(actionName);
    return action.activeness == 0;
}

bool InputManager::justDown(const std::string& actionName) const
{
    const auto& action = m_actions.at(actionName);
    return action.activeness > 0 && action.previousActiveness == 0;
}

bool InputManager::justUp(const std::string& actionName) const
{
    const auto& action = m_actions.at(actionName);
    return action.activeness == 0 && action.previousActiveness != 0;
}

bool InputManager::justDownUp(const std::string& actionName) const
{
    const auto& action = m_actions.at(actionName);
    return action.activeness == 0 && action.previousActiveness != 0 &&
           m_previousUpdatedBindings.find("action." + actionName) != m_previousUpdatedBindings.end();
}

bool InputManager::axisChanged(const std::string& axisName) const
{
    const auto& axis = m_axes.at(axisName);
    return axis.value != 0.f;
}

float InputManager::axis(const std::string& axisName) const
{
    const auto& axis = m_axes.at(axisName);
    return axis.value;
}

void InputManager::bindAction(const std::string& actionName, MouseButton mouseButton)
{
    m_actions[actionName].mouseButtons.emplace(mouseButton);
}

void InputManager::bindAction(const std::string& actionName, VrButton vrButton, VrDeviceType hand)
{
    m_actions[actionName].vrControllerButtons.emplace_back(VrControllerButton{hand, vrButton});
}

void InputManager::bindAction(const std::string& actionName, Key key)
{
    m_actions[actionName].keys.emplace_back(std::set({key}));
}

void InputManager::bindAction(const std::string& actionName, const std::set<Key>& keys)
{
    m_actions[actionName].keys.emplace_back(keys);
}

void InputManager::bindAxis(const std::string& axisName, InputAxis inputAxis)
{
    m_axes[axisName].inputAxes.emplace(inputAxis);
}

void InputManager::updateReset()
{
    if (!m_updatedBindings.empty()) {
        m_previousUpdatedBindings = m_updatedBindings;
        m_updatedBindings.clear();
    }

    for (auto& iAction : m_actions) {
        auto& action = iAction.second;
        action.previousActiveness = action.activeness;
    }

    for (auto& iAxis : m_axes) {
        auto& axis = iAxis.second;
        axis.value = 0.f;
    }
}

void InputManager::update(WsEvent& event)
{
    // Mouse buttons
    if (event.type == WsEventType::MouseButtonPressed) {
        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            if (action.mouseButtons.find(event.mouseButton.which) != action.mouseButtons.end()) {
                action.activeness += 1u;
                m_updatedBindings.emplace("action." + iAction.first);
            }
        }
    }
    else if (event.type == WsEventType::MouseButtonReleased) {
        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            if (action.mouseButtons.find(event.mouseButton.which) != action.mouseButtons.end() && action.activeness != 0u) {
                action.activeness -= 1u;
                m_updatedBindings.emplace("action." + iAction.first);
            }
        }
    }

    // Keys
    else if (event.type == WsEventType::KeyPressed) {
        m_keysPressed[event.key.which] = true;

        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            for (const auto& keys : action.keys) {
                if (keys.find(event.key.which) != keys.end()) {
                    if (keysPressed(keys)) {
                        action.activeness += 1u;
                        m_updatedBindings.emplace("action." + iAction.first);
                    }
                }
            }
        }
    }
    else if (event.type == WsEventType::KeyReleased) {
        m_keysPressed[event.key.which] = false;

        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            for (const auto& keys : action.keys) {
                if (keys.find(event.key.which) != keys.end() && action.activeness != 0u) {
                    action.activeness -= 1u;
                    m_updatedBindings.emplace("action." + iAction.first);
                }
            }
        }
    }

    // Mouse move
    else if (event.type == WsEventType::MouseMoved) {
        m_mouseCoordinates.x = event.mouseMove.x;
        m_mouseCoordinates.y = event.mouseMove.y;

        for (auto& iAxis : m_axes) {
            auto& axis = iAxis.second;
            if (axis.inputAxes.find(InputAxis::MouseX) != axis.inputAxes.end()) {
                axis.value += event.mouseMove.dx;
                m_updatedBindings.emplace("axis." + iAxis.first);
            }
            if (axis.inputAxes.find(InputAxis::MouseY) != axis.inputAxes.end()) {
                axis.value += event.mouseMove.dy;
                m_updatedBindings.emplace("axis." + iAxis.first);
            }
        }
    }

    // Mouse wheel
    else if (event.type == WsEventType::MouseWheelScrolled) {
        if (event.mouseWheel.which == MouseWheel::Vertical) {
            for (auto& iAxis : m_axes) {
                auto& axis = iAxis.second;
                if (axis.inputAxes.find(InputAxis::MouseWheelVertical) != axis.inputAxes.end()) {
                    axis.value += event.mouseWheel.delta;
                    m_updatedBindings.emplace("axis." + iAxis.first);
                }
            }
        }
        else if (event.mouseWheel.which == MouseWheel::Horizontal) {
            for (auto& iAxis : m_axes) {
                auto& axis = iAxis.second;
                if (axis.inputAxes.find(InputAxis::MouseWheelHorizontal) != axis.inputAxes.end()) {
                    axis.value += event.mouseWheel.delta;
                    m_updatedBindings.emplace("axis." + iAxis.first);
                }
            }
        }
    }
}

void InputManager::update(VrEvent& event)
{
    // VR buttons
    if (event.type == VrEventType::ButtonPressed) {
        VrControllerButton vrControllerButton;
        vrControllerButton.hand = event.button.hand;
        vrControllerButton.button = event.button.which;

        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            for (auto& actionVrControllerButton : action.vrControllerButtons) {
                if ((actionVrControllerButton.button == vrControllerButton.button)
                    && ((actionVrControllerButton.hand == VrDeviceType::UnknownHand)
                        || (actionVrControllerButton.hand == vrControllerButton.hand))) {
                    action.activeness += 1u;
                    m_updatedBindings.emplace("action." + iAction.first);
                }
            }
        }
    }
    else if (event.type == VrEventType::ButtonReleased) {
        VrControllerButton vrControllerButton;
        vrControllerButton.hand = event.button.hand;
        vrControllerButton.button = event.button.which;

        for (auto& iAction : m_actions) {
            auto& action = iAction.second;
            for (auto& actionVrControllerButton : action.vrControllerButtons) {
                if ((actionVrControllerButton.button == vrControllerButton.button)
                    && ((actionVrControllerButton.hand == VrDeviceType::UnknownHand)
                        || (actionVrControllerButton.hand == vrControllerButton.hand))) {
                    action.activeness -= 1u;
                    m_updatedBindings.emplace("action." + iAction.first);
                }
            }
        }
    }
}

bool InputManager::keysPressed(const std::set<Key>& keys) const
{
    for (auto key : keys) {
        if (m_keysPressed.find(key) == m_keysPressed.end() || !m_keysPressed.at(key)) {
            return false;
        }
    }
    return true;
}
