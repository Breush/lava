#include <lava/flow/sound.hpp>

#include <lava/flow/sound-data.hpp>

#if defined(LAVA_FLOW_AUDIO_PULSE)
#include "./backends/pulse/sound-impl.hpp"
#endif

using namespace lava::flow;

Sound::Sound(AudioEngine& engine, std::shared_ptr<SoundData> soundData)
{
    m_impl = new SoundImpl(engine.impl(), soundData);
}

Sound::Sound(AudioEngine& engine, const std::string& fileName)
    : Sound(engine, engine.share<SoundData>(fileName))
{
}

Sound::~Sound()
{
    delete m_impl;
}

$pimpl_method_cast(Sound, SoundBaseImpl, void, position, const glm::vec3&, position);
$pimpl_property_cast_v(Sound, SoundBaseImpl, float, cutOffDistance);
$pimpl_property_cast_v(Sound, SoundBaseImpl, float, spatializationHalfDistance);
