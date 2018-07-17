#pragma once

namespace lava::flow {
    /**
     * Base for playable audio.
     */
    class AudioSource {
    public:
        /// Plays the current sound data from the start.
        void play();

        /// Stops anything that is playing.
        void stop();

        /// Plays the current sound data once and remove this sound.
        void playOnce();

        /**
         * Whether the sound should be replayed at the end.
         * Not effective if playOnce is called.
         */
        void looping(bool looping);
        bool looping() const;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    protected:
        Impl* m_impl = nullptr;
    };
}
