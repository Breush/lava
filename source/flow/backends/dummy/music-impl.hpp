#pragma once

#include "../../music-base-impl.hpp"

namespace lava::flow {
    /**
     * Dummy implementation of Music class (doing nothing).
     */
    class MusicImpl : public MusicBaseImpl {
    public:
        MusicImpl(AudioEngine::Impl& engine, const std::shared_ptr<IMusicData>& /* musicData */)
            : MusicBaseImpl(engine)
        {
        }
        ~MusicImpl() {}

        // ----- AudioSource

        void update() final {}
        void restart() final {}
    };
}
