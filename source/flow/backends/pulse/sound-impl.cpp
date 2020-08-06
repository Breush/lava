#include "./sound-impl.hpp"

#include <lava/flow/sound-data.hpp>

#include "./audio-engine-impl.hpp"

using namespace lava::chamber;
using namespace lava::flow;

SoundImpl::SoundImpl(AudioEngine::Impl& engine, const std::shared_ptr<SoundData>& soundData)
    : SoundBaseImpl(engine, soundData)
    , m_backendEngine(engine.backend())
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    // @todo This 2 channels rule should be decided by the engine,
    // as we might want to manage surround system some day.

    // Global output is normalized whatever happens, to allow effects.
    pa_sample_spec sampleSpec;
    sampleSpec.format = PA_SAMPLE_FLOAT32;
    sampleSpec.channels = 2u;
    sampleSpec.rate = 44100;

    m_stream = pa_stream_new(m_backendEngine.context(), "lava.flow.sound", &sampleSpec, nullptr);
    pa_stream_connect_playback(m_stream, nullptr, nullptr, static_cast<pa_stream_flags_t>(0u), nullptr, nullptr);
}

SoundImpl::~SoundImpl()
{
    pa_stream_disconnect(m_stream);
    pa_stream_unref(m_stream);
}

// ----- AudioSource

void SoundImpl::update()
{
    if (pa_stream_get_state(m_stream) != PA_STREAM_READY) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    const auto writableSize = pa_stream_writable_size(m_stream);
    const auto remainingSize = (m_soundData->normalizedSize() - m_playingOffset) * sizeof(float);
    uint32_t playingSize = (remainingSize < writableSize) ? remainingSize : writableSize;

    // @note We only take long enough samples to be sure sound effects are all right.
    if (playingSize >= 512u || (playingSize > 0u && playingSize == remainingSize)) {
        const auto rawSoundPointer = m_soundData->normalizedData() + m_playingOffset;
        const auto size = static_cast<uint32_t>(playingSize / sizeof(float));
        Buffer buffer = {rawSoundPointer, size, size};
        buffer = applyEffects(buffer);

        pa_stream_write(m_stream, buffer.data, buffer.size * sizeof(float), nullptr, 0, PA_SEEK_RELATIVE);
        m_playingOffset += buffer.samplesCount;
    }

    // Check if we are at the end, and if so, just stop.
    if (remainingSize == 0u && pa_stream_get_underflow_index(m_stream) >= 0) {
        finish();
    }
}

void SoundImpl::restart()
{
    m_playingOffset = 0u;
}
