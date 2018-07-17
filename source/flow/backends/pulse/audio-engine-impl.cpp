#include "./audio-engine-impl.hpp"

#include <algorithm>
#include <lava/chamber.hpp>

#include "./music-impl.hpp"
#include "./sound-impl.hpp"

using namespace lava::flow;
using namespace lava::chamber;

AudioEngineImpl::AudioEngineImpl()
{
    // Main loop
    m_mainLoop = pa_mainloop_new();

    // Context
    m_context = pa_context_new(pa_mainloop_get_api(m_mainLoop), "lava-flow");
    pa_context_connect(m_context, nullptr, static_cast<pa_context_flags_t>(0u), nullptr);
    waitUntilContextReady();
}

AudioEngineImpl::~AudioEngineImpl()
{
    pa_context_disconnect(m_context);
    pa_context_unref(m_context);
    m_context = nullptr;

    pa_mainloop_free(m_mainLoop);
    m_mainLoop = nullptr;
}

// ----- AudioEngine::Impl

void AudioEngineImpl::internalUpdate()
{
    pa_mainloop_iterate(m_mainLoop, 0, nullptr);
}

//----- Internal

void AudioEngineImpl::waitUntilContextReady()
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
