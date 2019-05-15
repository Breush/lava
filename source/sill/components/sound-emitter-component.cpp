#include <lava/sill/components/sound-emitter-component.hpp>

#include "./sound-emitter-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(SoundEmitterComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(SoundEmitterComponent, void, update, float, dt);

// SoundEmitterComponent
$pimpl_method(SoundEmitterComponent, void, add, const std::string&, hrid, const std::string&, path);
$pimpl_method(SoundEmitterComponent, void, start, const std::string&, hrid);
