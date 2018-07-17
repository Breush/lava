#include "./sound-data-impl.hpp"

#include <lava/chamber/buffer.hpp>
#include <lava/chamber/logger.hpp>

using namespace lava;

namespace {
    float readAsFloat(const uint8_t* buffer, uint32_t offset, flow::SampleFormat sampleFormat)
    {
        switch (sampleFormat) {
        case flow::SampleFormat::Int16LE: return chamber::readInt16LE(buffer, offset) / 32768.f;
        default: {
            chamber::logger.error("flow.sound-data.sound-data") << "Unhandled sample format: " << sampleFormat << std::endl;
        }
        }

        return 0.f;
    }

    uint32_t sampleFormatBytesCount(flow::SampleFormat sampleFormat)
    {
        switch (sampleFormat) {
        case flow::SampleFormat::Int16LE: return sizeof(int16_t); break;
        default: {
            chamber::logger.error("flow.sound-data.sound-data") << "Unhandled sample format: " << sampleFormat << std::endl;
        }
        }

        return 0u;
    }
}

using namespace lava::flow;
using namespace lava::chamber;

void SoundData::Impl::normalize()
{
    if (m_size == 0u || m_channels == 0u) {
        m_normalizedData.resize(0u);
        return;
    }

    const auto sampleFormatSize = sampleFormatBytesCount(m_sampleFormat);
    const auto samplesPerChannel = m_size / sampleFormatSize / m_channels;
    m_normalizedData.resize(samplesPerChannel);

    // ----- Convert to float and average to mono

    // @note Averaging is the usual method,
    // even if it may cause problems in very specific cases
    // (L and R signals in opposite phases), it is good enough.
    for (auto s = 0u; s < samplesPerChannel; ++s) {
        // Compute average on interleaved channels
        auto averageValue = 0.f;
        for (auto c = 0u; c < m_channels; ++c) {
            averageValue += readAsFloat(m_data, (m_channels * s + c) * sampleFormatSize, m_sampleFormat);
        }
        averageValue /= m_channels;

        // Set it to the normalized data
        m_normalizedData[s] = averageValue;
    }

    // ----- @fixme Resample to 44100

    if (m_rate != 44100) {
        logger.error("flow.sound-data") << "Currently, only sounds with a rate of 44100Hz are handled." << std::endl;
    }
}
