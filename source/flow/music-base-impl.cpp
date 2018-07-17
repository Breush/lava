#include "./music-base-impl.hpp"

using namespace lava::flow;

MusicBaseImpl::MusicBaseImpl(AudioEngine::Impl& engine)
    : AudioSource::Impl(engine)
{
}
