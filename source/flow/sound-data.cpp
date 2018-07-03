#include <lava/flow/sound-data.hpp>

#include <lava/core/macros/pimpl.hpp>

#include "./sound-data/wave-sound-data-impl.hpp"

using namespace lava::flow;

SoundData::SoundData(const std::string& fileName)
{
    // @todo Have a switch based on file type.
    m_impl = new WaveSoundDataImpl(fileName);
}

SoundData::~SoundData()
{
    delete m_impl;
}

$pimpl_method_const(SoundData, const uint8_t*, data);
$pimpl_method_const(SoundData, uint32_t, size);
$pimpl_method_const(SoundData, uint32_t, rate);
$pimpl_method_const(SoundData, uint8_t, channels);
$pimpl_method_const(SoundData, SampleFormat, sampleFormat);
