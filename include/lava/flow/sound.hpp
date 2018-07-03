#pragma once

#include <memory>
#include <string>

namespace lava::flow {
    class AudioEngine;
    class SoundData;
}

namespace lava::flow {
    class Sound {
    public:
        /// Create a new sound based on preexisting SoundData.
        Sound(AudioEngine& engine, std::shared_ptr<SoundData> soundData);

        /// Create a new sound and associated SoundData, the data won't be sharable.
        Sound(AudioEngine& engine, const std::string& fileName);

        ~Sound();

        /// Plays the current sound data.
        void play();

        /// Plays the current sound data once and remove this sound.
        void playOnce();

        /**
         * Whether the sound should be replayed at the end.
         * Not effective if playOnce is called.
         */
        void looping(bool looping);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
