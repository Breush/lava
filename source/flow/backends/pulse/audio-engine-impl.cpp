#include "./audio-engine-impl.hpp"

#include <algorithm>
#include <lava/chamber.hpp>

#include "./music-impl.hpp"
#include "./sound-impl.hpp"

using namespace lava::flow;
using namespace lava::chamber;

AudioEngine::Impl::Impl()
{
    // Main loop
    m_mainLoop = pa_mainloop_new();

    // Context
    m_context = pa_context_new(pa_mainloop_get_api(m_mainLoop), "lava-flow");
    pa_context_connect(m_context, nullptr, static_cast<pa_context_flags_t>(0u), nullptr);
    waitUntilContextReady();
}

AudioEngine::Impl::~Impl()
{
    pa_context_disconnect(m_context);
    pa_context_unref(m_context);
    m_context = nullptr;

    pa_mainloop_free(m_mainLoop);
    m_mainLoop = nullptr;
}

void AudioEngine::Impl::update()
{
    // @todo We could take a delta-time or a speed factor as a parameter.

    for (const auto& sound : m_sounds) {
        if (sound->impl().playing()) {
            sound->impl().update();
        }
    }

    for (const auto& music : m_musics) {
        if (music->impl().playing()) {
            music->impl().update();
        }
    }

    pa_mainloop_iterate(m_mainLoop, 0, nullptr);

    // Automatically remove sounds that are marked to be removed...
    for (const auto* soundToRemove : m_soundsToRemove) {
        for (auto iSound = m_sounds.begin(); iSound != m_sounds.end(); ++iSound) {
            if (&(*iSound)->impl() == soundToRemove) {
                m_sounds.erase(iSound);
                break;
            }
        }
    }

    m_soundsToRemove.clear();
}

void AudioEngine::Impl::add(std::unique_ptr<Sound>&& sound)
{
    m_sounds.emplace_back(std::move(sound));
}

void AudioEngine::Impl::add(std::unique_ptr<Music>&& music)
{
    m_musics.emplace_back(std::move(music));
}

void AudioEngine::Impl::remove(const Sound::Impl& sound)
{
    m_soundsToRemove.emplace_back(&sound);
}

bool AudioEngine::Impl::playing() const
{
    for (const auto& sound : m_sounds) {
        if (sound->impl().playing()) {
            return true;
        }
    }

    return false;
}

//----- Internal

void AudioEngine::Impl::waitUntilContextReady()
{
    auto timeLimit = time(nullptr) + 5; // 5 seconds max waiting
    while (timeLimit >= time(nullptr)) {
        pa_mainloop_iterate(m_mainLoop, 0, nullptr);
        if (PA_CONTEXT_READY == pa_context_get_state(m_context)) {
            logger.info("flow.pulse.audio-engine") << "Context connected." << std::endl;
            return;
        }
    }

    logger.error("flow.pulse.audio-engine") << "Cannot connect to context." << std::endl;
}
