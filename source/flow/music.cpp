#include <lava/flow/music.hpp>

#if defined(LAVA_FLOW_AUDIO_PULSE)
#include "./backends/pulse/music-impl.hpp"
#else
#include "./backends/dummy/music-impl.hpp"
#endif

using namespace lava::flow;

Music::Music(AudioEngine& engine, const std::shared_ptr<IMusicData>& musicData)
{
    m_impl = new MusicImpl(engine.impl(), musicData);
}

Music::~Music()
{
    delete m_impl;
}
