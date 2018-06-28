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
        SoundFormat soundFormat() const { return m_soundFormat; }

    protected:
        uint8_t* m_data = nullptr;
        uint32_t m_size = 0u;

        uint32_t m_rate = 0u;
        uint8_t m_channels = 0u;
        SoundFormat m_soundFormat = SoundFormat::Unknown;
    };
}
