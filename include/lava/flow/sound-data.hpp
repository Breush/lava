#pragma once

#include <lava/flow/sample-format.hpp>

#include <string>

namespace lava::flow {
    class SoundData {
    public:
        SoundData(const std::string& fileName);
        ~SoundData();

        /// The raw data of the sample.
        const uint8_t* data() const;

        /// The size of the raw data.
        uint32_t size() const;

        /// Sample rate (in Hz).
        uint32_t rate() const;

        /// Sample channels (1 for mono, 2 for stereo and more if needed).
        uint8_t channels() const;

        /// Sample format.
        SampleFormat sampleFormat() const;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
