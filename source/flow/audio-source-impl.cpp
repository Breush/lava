#include "./audio-source-impl.hpp"

#include "./audio-engine-impl.hpp"

using namespace lava::flow;

AudioSource::Impl::Impl(AudioEngine::Impl& engine)
    : m_engine(engine)
{
}

void AudioSource::Impl::playing(bool playing)
{
    m_playing = playing;
    playing ? restart() : finish();
}

void AudioSource::Impl::looping(bool looping)
{
    m_looping = looping;
}

void AudioSource::Impl::removeOnFinish(bool removeOnFinish)
{
    m_removeOnFinish = removeOnFinish;
}

// ----- Sub-class API

void AudioSource::Impl::finish()
{
    m_playing = false;

    if (m_removeOnFinish) {
        m_engine.remove(*this);
    }
    else if (m_looping) {
        m_playing = true;
        restart();
    }
}
