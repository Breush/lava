#pragma once

#include "../../audio-engine-impl.hpp"

#include <pulse/pulseaudio.h>

namespace lava::flow {
    /**
     * PulseAudio implementation of AudioEngine.
     */
    class AudioEngineImpl : public AudioEngine::Impl {
    public:
        AudioEngineImpl();
        ~AudioEngineImpl();

        void internalUpdate() override final;

        // ----- Internal

        pa_mainloop* mainLoop() { return m_mainLoop; }
        pa_context* context() { return m_context; }

    protected:
        void waitUntilContextReady();

    private:
        pa_mainloop* m_mainLoop = nullptr;
        pa_context* m_context = nullptr;
    };
}
