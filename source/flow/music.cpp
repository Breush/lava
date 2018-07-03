#include <lava/flow/music.hpp>

#if defined(LAVA_FLOW_AUDIO_PULSE)
#include "./backends/pulse/music-impl.hpp"
#endif

using namespace lava::flow;

Music::Music(AudioEngine& engine, std::shared_ptr<IMusicData> musicData)
{
    m_impl = new Music::Impl(engine.impl(), musicData);
}

Music::~Music()
{
    delete m_impl;
}

void Music::play()
{
    m_impl->play();
}
