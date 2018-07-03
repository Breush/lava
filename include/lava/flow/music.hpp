#pragma once

#include <memory>

namespace lava::flow {
    class AudioEngine;
    class IMusicData;
}

namespace lava::flow {
    class Music {
    public:
        /// Create a new music.
        Music(AudioEngine& engine, std::shared_ptr<IMusicData> musicData);

        ~Music();

        /// Plays the current music.
        void play();

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
