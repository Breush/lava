#include "./vorbis-music-decoder.hpp"

#include <lava/chamber/logger.hpp>

using namespace lava::chamber;
using namespace lava::flow;

VorbisMusicDecoder::VorbisMusicDecoder(std::shared_ptr<IMusicData> musicData)
    : m_musicData(musicData)
{
    m_decoder = stb_vorbis_open_memory(musicData->data(), musicData->size(), nullptr, nullptr);

    auto vorbisInfo = stb_vorbis_get_info(m_decoder);
    m_channels = vorbisInfo.channels;
    m_rate = vorbisInfo.sample_rate;
    m_sampleFormat = SampleFormat::Float32;

    logger.info("flow.vorbis-music-decoder").tab(1);
    logger.log() << "Rate: " << m_rate << std::endl;
    logger.log() << "Channels: " << static_cast<uint32_t>(m_channels) << std::endl;
    logger.log() << "Sample format: " << m_sampleFormat << std::endl;
    logger.log().tab(-1);
}

VorbisMusicDecoder::~VorbisMusicDecoder()
{
    stb_vorbis_close(m_decoder);
}

//----- IMusicDecoder

void VorbisMusicDecoder::acquireNextFrame(uint32_t frameSize)
{
    m_buffer.resize(static_cast<uint32_t>(frameSize / sizeof(float)));

    auto writtenSamples = stb_vorbis_get_samples_float_interleaved(m_decoder, m_channels, m_buffer.data(), m_buffer.size());

    if (m_buffer.size() != static_cast<uint32_t>(m_channels * writtenSamples)) {
        m_buffer.resize(m_channels * writtenSamples);
    }
}

void VorbisMusicDecoder::seekStart()
{
    stb_vorbis_seek_start(m_decoder);
}
