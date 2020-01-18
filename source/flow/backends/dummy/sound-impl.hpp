#pragma once

#include "../../sound-base-impl.hpp"

namespace lava::flow {
    /**
     * Dummy implementation of sound class (doing nothing).
     */
    class SoundImpl : public SoundBaseImpl {
    public:
        SoundImpl(AudioEngine::Impl& engine, std::shared_ptr<SoundData> soundData)
            : SoundBaseImpl(engine, soundData)
        {
        }
        ~SoundImpl() {}

        // ----- AudioSource

        void update() final {}
        void restart() final {}
    };
}
