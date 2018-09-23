#pragma once

#include <lava/flow/audio-engine.hpp>

namespace lava::flow {
    class AudioEngineImpl;
}

namespace lava::flow {
    /**
     * Base for all AudioEngine::Impl implementations.
     *
     * Doesn't know anything about the backend.
     */
    class AudioEngine::Impl {
    public:
        virtual ~Impl() = default;

        // ----- AudioEngine

        void update();

        void add(std::unique_ptr<AudioSource>&& source);

        const glm::mat4& listenerTransform() const { return m_listenerTransform; }
        const glm::vec3& listenerPosition() const { return m_listenerPosition; }
        void listenerPosition(const glm::vec3& listenerPosition);

        // ----- Internal

        AudioEngineImpl& backend() { return reinterpret_cast<AudioEngineImpl&>(*this); }
        const AudioEngineImpl& backend() const { return reinterpret_cast<const AudioEngineImpl&>(*this); }

        void remove(const AudioSource::Impl& source);

    protected:
        virtual void internalUpdate() = 0;

    protected:
        void updateTransform();

    protected:
        // Resources
        std::vector<std::unique_ptr<AudioSource>> m_sources;

        std::vector<const AudioSource::Impl*> m_sourcesToRemove;

        // Spatialization
        glm::mat4 m_listenerTransform;
        glm::vec3 m_listenerPosition;
    };
}
