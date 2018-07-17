#pragma once

#include <lava/flow/audio-source.hpp>

#include <lava/flow/audio-engine.hpp>

namespace lava::flow {
    /**
     * Base for all sources implementations stored in the engine.
     */
    class AudioSource::Impl {
    public:
        Impl(AudioEngine::Impl& engine);
        virtual ~Impl() = default;

        /// Called by the AudioEngine::Impl.
        virtual void update() = 0;

        /// Restart playing.
        virtual void restart() = 0;

        /// Start playing the source (calls restart).
        void playing(bool playing);
        bool playing() const { return m_playing; }

        /// When the source ends, restart it.
        void looping(bool looping);
        bool looping() const { return m_looping; }

        /// When the source ends, remove it.
        void removeOnFinish(bool removeOnFinish);
        bool removeOnFinish() const { return m_removeOnFinish; }

    protected:
        // ----- Sub-class API

        /**
         * To be called by backend implementation when a playing has finished.
         * It will loop if needed.
         */
        void finish();

    protected:
        AudioEngine::Impl& m_engine;

        // User control
        bool m_playing = false;
        bool m_looping = false;
        bool m_removeOnFinish = false;
    };
}
