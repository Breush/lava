#pragma once

#include <lava/flow/sample-format.hpp>

#include <string>

namespace lava::flow {
    class SoundData {
    public:
        SoundData(const std::string& fileName);
        ~SoundData();

        /// The raw data (encoded interlaced according to sampleFormat).
        const uint8_t* data() const;

        /// The size of the raw data (in bytes).
        uint32_t size() const;

        /// Sample rate (in Hz).
        uint32_t rate() const;

        /// Sample channels (1 for mono, 2 for stereo and more if needed).
        uint8_t channels() const;

        /// Sample format.
        SampleFormat sampleFormat() const;

        /// The raw data converted to single channel, 44100Hz and 32-bit float PCM.
        const float* normalizedData() const;

        /// The size of normalizedData (floats count).
        uint32_t normalizedSize() const;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
