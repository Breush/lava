#pragma once

#include <lava/flow/audio-engine.hpp>
#include <lava/flow/music.hpp>

#include <pulse/pulseaudio.h>

#include "../../i-music-decoder.hpp"

namespace lava::flow {
    /**
     * PulseAudio implementation of Music class.
     */
    class Music::Impl {
    public:
        Impl(AudioEngine::Impl& engine, std::shared_ptr<IMusicData> musicData);
        ~Impl();

        // ----- Music

        void play();

        // ----- Internal

        void update();
        bool playing() const { return m_playing; }

    private:
        AudioEngine::Impl& m_engine;

        std::unique_ptr<IMusicDecoder> m_musicDecoder = nullptr;
        pa_stream* m_stream = nullptr;
        bool m_playing = false;

        uint32_t m_playingPointer = 0u;
    };
}
