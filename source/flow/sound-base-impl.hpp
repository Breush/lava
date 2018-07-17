#pragma once

#include <lava/flow/audio-engine.hpp>
#include <lava/flow/sound.hpp>

#include <memory>
#include <vector>

#include "./audio-source-impl.hpp"

namespace lava::flow {
    /**
     * Base for all SoundImpl implementations.
     *
     * Doesn't know anything about the backend,
     * but handles effects and such.
     */
    class SoundBaseImpl : public AudioSource::Impl {
    public:
        SoundBaseImpl(AudioEngine::Impl& engine, std::shared_ptr<SoundData> soundData);
        virtual ~SoundBaseImpl() = default;

    public:
        // ----- Sound

        float cutOffDistance() const { return m_cutOffDistance; }
        void cutOffDistance(float cutOffDistance) { m_cutOffDistance = cutOffDistance; }

        float spatializationHalfDistance() const { return m_spatializationHalfDistance; }
        void spatializationHalfDistance(float spatializationHalfDistance)
        {
            m_spatializationHalfDistance = spatializationHalfDistance;
        }

        const glm::vec3& position() const { return m_position; }
        void position(const glm::vec3& position);

    protected:
        // ----- Sub-class API

        struct Buffer {
            const float* data = nullptr; // Pointer to the buffer data.
            uint32_t size = 0u;          // Size of buffer data (number of floats).
            uint32_t samplesCount = 0u;  // How many samples are used to make this signal.
        };

        /**
         * Will apply all sound effects (if any) to the provided (normalized) buffer
         * and generate a new one (with 2 channels and all effects applied).
         */
        Buffer applyEffects(Buffer buffer);

    private:
        // ----- Internal

        void applyMonoEffects();

        /// The further, the silencer. Based on cutOffDistance.
        bool applyDistanceAttenuationEffect();

        void applyStereoEffects();
        bool applySpatializationEffect();

    protected:
        // References
        std::shared_ptr<SoundData> m_soundData;

        // User control
        float m_cutOffDistance = 100.f;
        float m_spatializationHalfDistance = 10.f;

        // Effects
        bool m_effectsEnabled = false;
        std::vector<float> m_buffer;
        uint32_t m_bufferSamplesCount;

        // Spatialization
        glm::vec3 m_position;
    };
}
