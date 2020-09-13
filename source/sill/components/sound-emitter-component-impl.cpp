#include "./sound-emitter-component-impl.hpp"

#include <lava/sill/game-engine.hpp>

using namespace lava::sill;

SoundEmitterComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
    , m_audioEngine(m_entity.engine().audioEngine())
{
}

// ----- SoundEmitterComponent

void SoundEmitterComponent::Impl::add(const std::string& hrid, const std::string& path)
{
    m_soundsData[hrid] = m_audioEngine.share<flow::SoundData>(path);
}

void SoundEmitterComponent::Impl::start(const std::string& hrid)
{
    auto& sound = m_audioEngine.make<flow::Sound>(m_soundsData[hrid]);
    sound.playOnce();
}
