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

bool InputManager::Impl::axisChanged(const std::string& axisName) const
{
    const auto& axis = m_axes.at(axisName);
    return axis.value != 0.f;
}

float InputManager::Impl::axis(const std::string& axisName) const
{
    const auto& axis = m_axes.at(axisName);
    return axis.value;
}

void InputManager::Impl::bindAction(const std::string& actionName, MouseButton mouseButton)
{
    m_actions[actionName].mouseButtons.emplace(mouseButton);
}

void InputManager::Impl::bindAction(const std::string& actionName, Key key)
{
    m_actions[actionName].keys.emplace(key);
}

void InputManager::Impl::bindAxis(const std::string& axisName, InputAxis inputAxis)
{
    m_axes[axisName].inputAxes.emplace(inputAxis);
}

void InputManager::Impl::updateReset()
{
    for (auto& iAction : m_actions) {
        auto& action = iAction.second;
        action.previousActiveness = action.activeness;
    }

    for (auto& iAxis : m_axes) {
        auto& axis = iAxis.second;
        axis.value = 0.f;
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

    // Mouse move
    else if (event.type == WsEventType::MouseMoved) {
        // @note To prevent big deltas when moving
        // the mouse the first time, we use this flag.
        if (m_initializingMousePosition) {
            m_mousePosition.x = event.mouseMove.x;
            m_mousePosition.y = event.mouseMove.y;
            m_initializingMousePosition = false;
            return;
        }

        for (auto& iAxis : m_axes) {
            auto& axis = iAxis.second;
            if (axis.inputAxes.find(InputAxis::MouseX) != axis.inputAxes.end()) {
                axis.value += event.mouseMove.x - m_mousePosition.x;
            }
            if (axis.inputAxes.find(InputAxis::MouseY) != axis.inputAxes.end()) {
                axis.value += event.mouseMove.y - m_mousePosition.y;
            }
        }

        m_mousePosition.x = event.mouseMove.x;
        m_mousePosition.y = event.mouseMove.y;
    }

    // Mouse wheel
    else if (event.type == WsEventType::MouseWheelScrolled) {
        if (event.mouseWheel.which == MouseWheel::Vertical) {
            for (auto& iAxis : m_axes) {
                auto& axis = iAxis.second;
                if (axis.inputAxes.find(InputAxis::MouseWheelVertical) != axis.inputAxes.end()) {
                    axis.value += event.mouseWheel.delta;
                }
            }
        }
        else if (event.mouseWheel.which == MouseWheel::Horizontal) {
            for (auto& iAxis : m_axes) {
                auto& axis = iAxis.second;
                if (axis.inputAxes.find(InputAxis::MouseWheelHorizontal) != axis.inputAxes.end()) {
                    axis.value += event.mouseWheel.delta;
                }
            }
        }
    }
}
