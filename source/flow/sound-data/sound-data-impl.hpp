#pragma once

#include <lava/flow/sound-data.hpp>

namespace lava::flow {
    class SoundData::Impl {
    public:
        Impl() = default;
        virtual ~Impl() = default;

        // ----- SoundData

        const uint8_t* data() const { return m_data; }
        uint32_t size() const { return m_size; }

        uint32_t rate() const { return m_rate; }
        uint8_t channels() const { return m_channels; }
        SampleFormat sampleFormat() const { return m_sampleFormat; }

        const float* normalizedData() const { return m_normalizedData.data(); }
        uint32_t normalizedSize() const { return m_normalizedData.size(); }

        // ----- Sub-class API

        /**
         * Fill normalizedData according to current data.
         *
         * The normalized data will be single-channel, 44100Hz and 32-bit float.
         */
        void normalize();

    protected:
        uint8_t* m_data = nullptr;
        uint32_t m_size = 0u;

        uint32_t m_rate = 0u;
        uint8_t m_channels = 0u;
        SampleFormat m_sampleFormat = SampleFormat::Unknown;

        std::vector<float> m_normalizedData;
    };
}
