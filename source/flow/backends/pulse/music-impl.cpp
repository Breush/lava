#include "./music-impl.hpp"

#include <lava/flow/i-music-data.hpp>

#include "../../music-decoders/vorbis-music-decoder.hpp"
#include "./audio-engine-impl.hpp"
#include "./sample-helper.hpp"

using namespace lava::chamber;
using namespace lava::flow;

MusicImpl::MusicImpl(AudioEngine::Impl& engine, std::shared_ptr<IMusicData> musicData)
    : MusicBaseImpl(engine)
    , m_backendEngine(engine.backend())
{
    // @todo This decoder selection could be done upstream in Music()
    if (musicData->compressionFormat() == MusicCompressionFormat::Vorbis) {
        m_musicDecoder = std::make_unique<VorbisMusicDecoder>(musicData);
    }
    else {
        logger.error("flow.pulse.music") << "Unhandled compression format: " << musicData->compressionFormat() << std::endl;
    }

    pa_sample_spec sampleSpec;
    sampleSpec.format = helpers::pulseSampleFormat(m_musicDecoder->sampleFormat());
    sampleSpec.channels = m_musicDecoder->channels();
    sampleSpec.rate = m_musicDecoder->rate();

    m_stream = pa_stream_new(m_backendEngine.context(), "lava.flow.music", &sampleSpec, nullptr);
    pa_stream_connect_playback(m_stream, nullptr, nullptr, static_cast<pa_stream_flags_t>(0u), nullptr, nullptr);
}

MusicImpl::~MusicImpl()
{
    pa_stream_disconnect(m_stream);
    pa_stream_unref(m_stream);
}

// ----- AudioSource

void MusicImpl::update()
{
    if (pa_stream_get_state(m_stream) != PA_STREAM_READY) return;

    const auto writableSize = pa_stream_writable_size(m_stream);
    if (writableSize == 0u) return;

    if (m_playingOffset >= m_musicDecoder->frameSize()) {
        m_musicDecoder->acquireNextFrame(writableSize);
        m_playingOffset = 0u;

        // Check if we are at the end, and if so, just stop.
        if (m_musicDecoder->frameSize() == 0u) {
            m_playing = false;
            return;
        }
    }

    const auto remainingSize = m_musicDecoder->frameSize() - m_playingOffset;
    const auto playingSize = (remainingSize < writableSize) ? remainingSize : writableSize;

    if (playingSize > 0) {
        pa_stream_write(m_stream, m_musicDecoder->frameData() + m_playingOffset, playingSize, nullptr, 0, PA_SEEK_RELATIVE);
        m_playingOffset += playingSize;
    }
}

void MusicImpl::restart()
{
    m_playingOffset = 0u;
}
