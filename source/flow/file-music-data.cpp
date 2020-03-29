#include <lava/flow/music-data/file-music-data.hpp>

using namespace lava::flow;
using namespace lava::chamber;

FileMusicData::FileMusicData(const std::string& fileName)
{
    logger.info("flow.file-music-data").tab(1) << "Reading '" << fileName << "'." << std::endl;
    logger.log().tab(1);

    std::ifstream file(fileName, std::ifstream::binary);

    if (!file.is_open()) {
        logger.error("flow.file-music-data") << "Cannot open file." << std::endl;
    }

    m_data.insert(m_data.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    // Check magic number
    if ((m_data.size() > 4u) && (m_data[0] == 'O' && m_data[1] == 'g' && m_data[2] == 'g' && m_data[3] == 'S')) {
        m_compressionFormat = MusicCompressionFormat::Vorbis;
    }
    else {
        logger.error("flow.file-music-data") << "Compression format is not recognized." << std::endl;
    }

    logger.log() << "CompressionFormat: " << m_compressionFormat << std::endl;

    logger.log().tab(-2);
}
