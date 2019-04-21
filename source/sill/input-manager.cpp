#include <lava/sill/input-manager.hpp>

#include "./input-manager-impl.hpp"

using namespace lava::sill;

$pimpl_class(InputManager);

$pimpl_method_const(InputManager, bool, down, const std::string&, actionName);
$pimpl_method_const(InputManager, bool, up, const std::string&, actionName);
$pimpl_method_const(InputManager, bool, justDown, const std::string&, actionName);
$pimpl_method_const(InputManager, bool, justUp, const std::string&, actionName);
$pimpl_method_const(InputManager, bool, axisChanged, const std::string&, axisName);
$pimpl_method_const(InputManager, float, axis, const std::string&, axisName);

$pimpl_method(InputManager, void, bindAction, const std::string&, actionName, MouseButton, mouseButton);
$pimpl_method(InputManager, void, bindAction, const std::string&, actionName, VrButton, vrButton, VrDeviceType, hand);
$pimpl_method(InputManager, void, bindAction, const std::string&, actionName, Key, key);
$pimpl_method(InputManager, void, bindAxis, const std::string&, axisName, InputAxis, inputAxis);

$pimpl_method(InputManager, void, updateReset);
$pimpl_method(InputManager, void, update, WsEvent&, event);
$pimpl_method(InputManager, void, update, VrEvent&, event);
