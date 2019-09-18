#include <lava/sill/components/sound-emitter-component.hpp>

#include "./sound-emitter-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(SoundEmitterComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(SoundEmitterComponent, void, update, float, dt);

// SoundEmitterComponent
void SoundEmitterComponent::add(const std::string& hrid, const std::string& path)
{
    m_sounds[hrid] = path;
    m_impl->add(hrid, path);
}

$pimpl_method(SoundEmitterComponent, void, start, const std::string&, hrid);
