#include <lava/flow/audio-engine.hpp>

#if defined(LAVA_FLOW_AUDIO_PULSE)
#include "./backends/pulse/audio-engine-impl.hpp"
#endif

using namespace lava::flow;

AudioEngine::AudioEngine()
{
    m_impl = new AudioEngine::Impl();
}

AudioEngine::~AudioEngine()
{
    delete m_impl;
}

void AudioEngine::update()
{
    m_impl->update();
}

bool AudioEngine::playing() const
{
    return m_impl->playing();
}

void AudioEngine::add(std::unique_ptr<Sound>&& sound)
{
    m_impl->add(std::move(sound));
}
