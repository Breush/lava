#include <lava/flow/sound.hpp>

#include <lava/flow/sound-data.hpp>
#include <memory>

#if defined(LAVA_FLOW_AUDIO_PULSE)
#include "./backends/pulse/sound-impl.hpp"
#endif

using namespace lava::flow;

Sound::Sound(AudioEngine& engine, std::shared_ptr<SoundData> soundData)
{
    m_impl = new Sound::Impl(engine.impl(), soundData);
}

Sound::Sound(AudioEngine& engine, const std::string& fileName)
    : Sound(engine, engine.share<SoundData>(fileName))
{
}

Sound::~Sound()
{
    delete m_impl;
}

void Sound::play()
{
    m_impl->play();
}

void Sound::playOnce()
{
    m_impl->play();
    m_impl->removeOnFinish(true);
}

void Sound::looping(bool looping)
{
    m_impl->looping(looping);
}
