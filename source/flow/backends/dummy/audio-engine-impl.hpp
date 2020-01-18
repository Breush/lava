#pragma once

#include "../../audio-engine-impl.hpp"

namespace lava::flow {
    /**
     * Dummy implementation of AudioEngine (doing nothing).
     */
    class AudioEngineImpl : public AudioEngine::Impl {
    public:
        AudioEngineImpl() {}
        ~AudioEngineImpl() {}

        void internalUpdate() final {}
    };
}
