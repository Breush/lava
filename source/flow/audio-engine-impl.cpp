#include "./audio-engine-impl.hpp"

#include "./audio-source-impl.hpp"

using namespace lava::flow;

void AudioEngine::Impl::update()
{
    // @todo We could take a delta-time or a speed factor as a parameter.

    for (const auto& source : m_sources) {
        if (source->impl().playing()) {
            source->impl().update();
        }
    }

    internalUpdate();

    // Automatically remove sources that are marked to be removed.
    for (const auto* sourceToRemove : m_sourcesToRemove) {
        for (auto iSource = m_sources.begin(); iSource != m_sources.end(); ++iSource) {
            if (&(*iSource)->impl() == sourceToRemove) {
                m_sources.erase(iSource);
                break;
            }
        }
    }

    m_sourcesToRemove.clear();
}

void AudioEngine::Impl::add(std::unique_ptr<AudioSource>&& source)
{
    m_sources.emplace_back(std::move(source));
}

void AudioEngine::Impl::remove(const AudioSource::Impl& source)
{
    m_sourcesToRemove.emplace_back(&source);
}

void AudioEngine::Impl::listenerPosition(const glm::vec3& listenerPosition)
{
    m_listenerPosition = listenerPosition;
    updateTransform();
}

void AudioEngine::Impl::updateTransform()
{
    m_listenerTransform = glm::mat4(1.f);
    m_listenerTransform = glm::translate(m_listenerTransform, m_listenerPosition);
}
