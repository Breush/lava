#pragma once

#include <lava/flow/audio-source.hpp>

#include <glm/vec3.hpp>
#include <memory>
#include <string>

namespace lava::flow {
    class AudioEngine;
    class SoundData;
}

namespace lava::flow {
    class Sound : public AudioSource {
    public:
        /// Create a new sound based on preexisting SoundData.
        Sound(AudioEngine& engine, const std::shared_ptr<SoundData>& soundData);

        /// Create a new sound and associated SoundData, the data won't be sharable.
        Sound(AudioEngine& engine, const std::string& fileName);

        ~Sound();

        /**
         * @name Spatialization
         */
        /// @{
        void position(const glm::vec3& position);

        /// The distance to the listener above which the sound is pure silence. (Default: 100.f)
        float cutOffDistance() const;
        void cutOffDistance(float cutOffDistance);

        /// The distance at which left or right output is exactly half. (Default: 10.f)
        float spatializationHalfDistance() const;
        void spatializationHalfDistance(float spatializationHalfDistance);
        /// @}
    };
}
