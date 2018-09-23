#pragma once

#include "../../music-base-impl.hpp"

#include <lava/flow/audio-engine.hpp>

#include "../../audio-engine-impl.hpp"
#include "../../i-music-decoder.hpp"

namespace lava::flow {
    /**
     * PulseAudio implementation of Music class.
     */
    class MusicImpl : public MusicBaseImpl {
    public:
        MusicImpl(AudioEngine::Impl& engine, std::shared_ptr<IMusicData> musicData);
        ~MusicImpl();

        // ----- AudioSource

        void update() override final;
        void restart() override final;

    private:
        AudioEngineImpl& m_backendEngine;

        std::unique_ptr<IMusicDecoder> m_musicDecoder = nullptr;
        pa_stream* m_stream = nullptr;

        uint32_t m_playingOffset = 0u;
    };
}
