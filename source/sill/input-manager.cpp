#include <lava/sill/input-manager.hpp>

#include <lava/chamber/macros.hpp>

#include "./input-manager-impl.hpp"

using namespace lava::sill;

$pimpl_class(InputManager);

$pimpl_method_const(InputManager, bool, justDown, const std::string&, actionName);

$pimpl_method(InputManager, void, bindAction, const std::string&, actionName, crater::input::Button, button);
$pimpl_method(InputManager, void, bindAction, const std::string&, actionName, crater::input::Key, key);

$pimpl_method(InputManager, void, updateReset);
$pimpl_method(InputManager, void, update, crater::Event&, event);
