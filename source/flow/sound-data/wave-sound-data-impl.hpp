#include "./sound-data-impl.hpp"

namespace lava::flow {
    class WaveSoundDataImpl : public SoundData::Impl {
    public:
        WaveSoundDataImpl(const std::string& fileName);

    private:
        std::vector<uint8_t> m_fileData;
    };
}
