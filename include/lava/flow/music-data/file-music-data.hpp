#pragma once

#include <lava/flow/i-music-data.hpp>

#include <string>
#include <vector>

namespace lava::flow {
    /**
     * Basic MusicData builder from a fileName.
     */
    class FileMusicData : public IMusicData {
    public:
        FileMusicData(const std::string& fileName);

        const uint8_t* data() const override final { return m_data.data(); }
        uint32_t size() const override final { return m_data.size(); }
        MusicCompressionFormat compressionFormat() const override final { return m_compressionFormat; };

    private:
        /// The raw data of the music file.
        std::vector<uint8_t> m_data;

        /// Compression format of the data.
        MusicCompressionFormat m_compressionFormat = MusicCompressionFormat::Unknown;
    };
}
