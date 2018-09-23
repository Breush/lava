#include <lava/flow/audio-source.hpp>

#include "./audio-source-impl.hpp"

using namespace lava::flow;

void AudioSource::play()
{
    m_impl->playing(true);
}

void AudioSource::stop()
{
    m_impl->playing(false);
}

void AudioSource::playOnce()
{
    m_impl->removeOnFinish(true);
    m_impl->playing(true);
}

$pimpl_property_v(AudioSource, bool, looping);
