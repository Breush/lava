#include "./wave-sound-data-impl.hpp"

using namespace lava::chamber;
using namespace lava::flow;

WaveSoundDataImpl::WaveSoundDataImpl(const std::string& fileName)
{
    logger.info("flow.wave-sound-data") << "Reading '" << fileName << "'." << std::endl;
    logger.log().tab(1);

    std::ifstream file(fileName, std::ifstream::binary);

    if (!file.is_open()) {
        logger.error("flow.wave-sound-data") << "Cannot open file." << std::endl;
    }

    m_fileData.insert(m_fileData.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    // Check header
    if ((m_fileData.size() < 44u)
        || (m_fileData[0] != 'R' || m_fileData[1] != 'I' || m_fileData[2] != 'F' || m_fileData[3] != 'F')
        || (m_fileData[8] != 'W' || m_fileData[9] != 'A' || m_fileData[10] != 'V' || m_fileData[11] != 'E')) {
        logger.error("flow.wave-sound-data") << "File is not a Wave one." << std::endl;
    }

    // Check format
    auto audioFormat = readUint16LE(m_fileData.data(), 20);
    if (audioFormat != 1u) {
        logger.error("flow.wave-sound-data") << "Unknown audio format " << audioFormat << "." << std::endl;
    }

    // Sample specification
    m_channels = readUint16LE(m_fileData.data(), 22);
    m_rate = readUint32LE(m_fileData.data(), 24);

    auto bitsPerSample = readUint16LE(m_fileData.data(), 34);
    switch (bitsPerSample) {
    case 8u: m_sampleFormat = SampleFormat::Uint8; break;
    case 16u: m_sampleFormat = SampleFormat::Int16LE; break;
    case 32u: m_sampleFormat = SampleFormat::Int32LE; break;
    default: logger.error("flow.wave-sound-data") << "Unknown bitsPerSample value." << std::endl;
    }

    logger.log() << "Channels: " << static_cast<uint32_t>(m_channels) << std::endl;
    logger.log() << "Rate: " << m_rate << std::endl;
    logger.log() << "Sample format: " << m_sampleFormat << std::endl;

    // Set-up data region
    // @note This 44 is the size of the header of Wave format.
    m_data = m_fileData.data() + 44u;
    m_size = m_fileData.size() - 44u;

    logger.log().tab(-1);

    normalize();
}
