#pragma once

#include <lava/flow/audio-engine.hpp>
#include <lava/flow/sound.hpp>

#include <memory>
#include <pulse/pulseaudio.h>
#include <vector>

namespace lava::flow {
    /**
     * PulseAudio implementation of sound class.
     */
    class Sound::Impl {
    public:
        Impl(AudioEngine::Impl& engine, std::shared_ptr<SoundData> soundData);
        ~Impl();

        // ----- Sound

        void play();
        void looping(bool looping);

        // ----- ISoundImpl

        void removeOnFinish(bool removeOnFinish);

        // ----- Internal

        void update();
        bool playing() const { return m_playing; }

    private:
        AudioEngine::Impl& m_engine;
        std::shared_ptr<SoundData> m_soundData;

        pa_stream* m_stream = nullptr;
        uint32_t m_playingPointer = 0u;
        bool m_playing = false;
        bool m_looping = false;
        bool m_removeOnFinish = false;
    };
}
