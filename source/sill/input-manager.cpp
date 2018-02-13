#include <lava/sill/input-manager.hpp>

#include <lava/chamber/macros.hpp>

#include "./input-manager-impl.hpp"

using namespace lava::sill;

$pimpl_class(InputManager);

$pimpl_method_const(InputManager, bool, justDown, const std::string&, actionName);

$pimpl_method(InputManager, void, bindAction, const std::string&, actionName, MouseButton, mouseButton);
$pimpl_method(InputManager, void, bindAction, const std::string&, actionName, Key, key);

$pimpl_method(InputManager, void, updateReset);
$pimpl_method(InputManager, void, update, WsEvent&, event);
