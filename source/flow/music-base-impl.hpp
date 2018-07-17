#pragma once

#include <lava/flow/music.hpp>

#include "./audio-source-impl.hpp"

namespace lava::flow {
    /**
     * Base for all MusicImpl implementations.
     */
    class MusicBaseImpl : public AudioSource::Impl {
    public:
        MusicBaseImpl(AudioEngine::Impl& engine);
    };
}
