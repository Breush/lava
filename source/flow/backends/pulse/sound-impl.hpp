#pragma once

#include <lava/flow/audio-engine.hpp>

#include "../../audio-engine-impl.hpp"
#include "../../sound-base-impl.hpp"

namespace lava::flow {
    /**
     * PulseAudio implementation of sound class.
     */
    class SoundImpl : public SoundBaseImpl {
    public:
        SoundImpl(AudioEngine::Impl& engine, std::shared_ptr<SoundData> soundData);
        ~SoundImpl();

        // ----- AudioSource

        void update() override final;
        void restart() override final;

    private:
        AudioEngineImpl& m_backendEngine;

        // Stream
        pa_stream* m_stream = nullptr;

        uint32_t m_playingOffset = 0u; // Number of floats we've already read.
    };
}
