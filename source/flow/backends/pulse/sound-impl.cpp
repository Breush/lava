#include "./sound-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <lava/flow/sound-data.hpp>

#include "./audio-engine-impl.hpp"
#include "./sample-helper.hpp"

using namespace lava::flow;

Sound::Impl::Impl(AudioEngine::Impl& engine, std::shared_ptr<SoundData> soundData)
    : m_engine(engine)
    , m_soundData(soundData)
{
    pa_sample_spec sampleSpec;
    sampleSpec.format = helpers::pulseSampleFormat(soundData->sampleFormat());
    sampleSpec.channels = soundData->channels();
    sampleSpec.rate = soundData->rate();

    // @todo Do we have to generate a unique name?
    m_stream = pa_stream_new(m_engine.context(), "lava.flow.sound", &sampleSpec, nullptr);
    pa_stream_connect_playback(m_stream, nullptr, nullptr, static_cast<pa_stream_flags_t>(0u), nullptr, nullptr);

    // @fixme We should probably create only one stream and mix the sounds by ourselves.
    // This will be needed for spacialized sound anyway.
}

Sound::Impl::~Impl()
{
    pa_stream_disconnect(m_stream);
    pa_stream_unref(m_stream);
}

void Sound::Impl::play()
{
    m_playing = true;

    // Restart playing from the start
    m_playingPointer = 0u;
}

void Sound::Impl::removeOnFinish(bool removeOnFinish)
{
    m_removeOnFinish = removeOnFinish;
}

// ----- Internal

void Sound::Impl::update()
{
    if (pa_stream_get_state(m_stream) != PA_STREAM_READY) return;

    const auto writableSize = pa_stream_writable_size(m_stream);
    const auto remainingSize = m_soundData->size() - m_playingPointer;
    const auto playingSize = (remainingSize < writableSize) ? remainingSize : writableSize;

    if (playingSize > 0) {
        pa_stream_write(m_stream, m_soundData->data() + m_playingPointer, playingSize, nullptr, 0, PA_SEEK_RELATIVE);
        m_playingPointer += playingSize;
    }

    // Check if we are at the end, and if so, just stop.
    if (m_playingPointer >= m_soundData->size() && pa_stream_get_underflow_index(m_stream) >= 0) {
        m_playing = false;

        if (m_removeOnFinish) {
            m_engine.remove(*this);
        }
    }
}
