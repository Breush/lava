#include <lava/flow/audio-engine.hpp>

#if defined(LAVA_FLOW_AUDIO_PULSE)
#include "./backends/pulse/audio-engine-impl.hpp"
#endif

using namespace lava::flow;

AudioEngine::AudioEngine()
{
    m_impl = new AudioEngineImpl();
}

AudioEngine::~AudioEngine()
{
    delete m_impl;
}

$pimpl_method(AudioEngine, void, update);

void AudioEngine::add(std::unique_ptr<AudioSource>&& source)
{
    m_impl->add(std::move(source));
}

$pimpl_method(AudioEngine, void, listenerPosition, const glm::vec3&, listenerPosition);
