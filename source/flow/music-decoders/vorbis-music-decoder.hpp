#pragma once

#include "../i-music-decoder.hpp"

#include <lava/flow/i-music-data.hpp>

#include <lava/chamber/stb/vorbis.hpp>

namespace lava::flow {
    /**
     * Ogg Vorbis implementation of music decoder.
     */
    class VorbisMusicDecoder : public IMusicDecoder {
    public:
        VorbisMusicDecoder(std::shared_ptr<IMusicData> musicData);
        ~VorbisMusicDecoder();

        //----- IMusicDecoder
        uint32_t rate() const override final { return m_rate; };
        uint8_t channels() const override final { return m_channels; };
        SampleFormat sampleFormat() const override final { return m_sampleFormat; };

        uint32_t frameSize() const override final { return m_buffer.size() * sizeof(float); }
        const uint8_t* frameData() const override final { return reinterpret_cast<const uint8_t*>(m_buffer.data()); }
        void acquireNextFrame(uint32_t frameSize) override final;

        void seekStart() override final;

    private:
        std::shared_ptr<IMusicData> m_musicData;

        uint32_t m_rate = 0u;
        uint8_t m_channels = 0u;
        SampleFormat m_sampleFormat = SampleFormat::Unknown;

        stb_vorbis* m_decoder = nullptr;
        std::vector<float> m_buffer;
    };
}
