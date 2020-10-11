#include <lava/sill/components/sound-emitter-component.hpp>

#include <lava/sill/entity.hpp>
#include <lava/sill/game-engine.hpp>

using namespace lava::sill;

SoundEmitterComponent::SoundEmitterComponent(Entity& entity)
    : IComponent(entity)
    , m_audioEngine(m_entity.engine().audioEngine())
{
}

void SoundEmitterComponent::add(const std::string& hrid, const std::string& path)
{
    auto& soundInfo = m_soundsInfos[hrid];
    soundInfo.path = path;
    soundInfo.soundData = m_audioEngine.share<flow::SoundData>(path);
}

void SoundEmitterComponent::start(const std::string& hrid) const
{
    auto& sound = m_audioEngine.make<flow::Sound>(m_soundsInfos.at(hrid).soundData);
    sound.playOnce();
}
