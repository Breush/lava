#pragma once

#include <lava/flow/audio-engine.hpp>

#include <memory>
#include <pulse/pulseaudio.h>
#include <vector>

namespace lava::flow {
    /**
     * PulseAudio implementation of AudioEngine.
     */
    class AudioEngine::Impl {
    public:
        Impl();
        ~Impl();

        void update();
        void add(std::unique_ptr<Sound>&& sound);
        void add(std::unique_ptr<Music>&& music);
        void remove(const Sound::Impl& sound);
        bool playing() const;

        // ----- Internal

        pa_mainloop* mainLoop() { return m_mainLoop; }
        pa_context* context() { return m_context; }

    protected:
        void waitUntilContextReady();

    private:
        pa_mainloop* m_mainLoop = nullptr;
        pa_context* m_context = nullptr;

        // @fixme There is a lot of non-pulse related code here,
        // we could extract that somehow to a AudioEngineImpl.
        std::vector<std::unique_ptr<Sound>> m_sounds;
        std::vector<std::unique_ptr<Music>> m_musics;
        std::vector<const Sound::Impl*> m_soundsToRemove;
    };
}
