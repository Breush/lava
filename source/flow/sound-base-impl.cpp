#include "./sound-base-impl.hpp"

#include "./audio-engine-impl.hpp"

using namespace lava::flow;
using namespace lava::chamber;

SoundBaseImpl::SoundBaseImpl(AudioEngine::Impl& engine, const std::shared_ptr<SoundData>& soundData)
    : AudioSource::Impl(engine)
    , m_soundData(soundData)
{
}

// ----- Sound

void SoundBaseImpl::position(const glm::vec3& position)
{
    m_effectsEnabled = true;
    m_position = position;
}

// ----- Sub-class API

SoundBaseImpl::Buffer SoundBaseImpl::applyEffects(Buffer buffer)
{
    // Copy the data so we can modify them!
    m_buffer = std::vector<float>(buffer.data, buffer.data + buffer.size);
    m_bufferSamplesCount = buffer.samplesCount;

    applyMonoEffects();

    // Mono -> stereo
    auto oldBufferSize = static_cast<int32_t>(m_buffer.size());
    m_buffer.resize(oldBufferSize * 2u);
    for (int32_t i = oldBufferSize - 1; i >= 0; --i) {
        m_buffer[2 * i + 1] = m_buffer[i];
        m_buffer[2 * i] = m_buffer[i];
    }

    applyStereoEffects();

    return {m_buffer.data(), static_cast<uint32_t>(m_buffer.size()), m_bufferSamplesCount};
}

// ----- Internal

void SoundBaseImpl::applyMonoEffects()
{
    if (!m_effectsEnabled) return;

    if (applyDistanceAttenuationEffect()) return;

    // @todo Doppler effect
}

bool SoundBaseImpl::applyDistanceAttenuationEffect()
{
    auto listenerPosition = m_engine.listenerPosition();
    auto soundDistance = glm::distance(listenerPosition, m_position);
    auto localIntensity = soundDistance / m_cutOffDistance;
    auto distanceAttenuation = 1.f - localIntensity * localIntensity;

    // Pure silence.
    if (distanceAttenuation < 0.f) {
        distanceAttenuation = 0.f;
    }

    for (auto i = 0u; i < m_buffer.size(); ++i) {
        m_buffer[i] = distanceAttenuation * m_buffer[i];
    }

    return distanceAttenuation == 0.f;
}

void SoundBaseImpl::applyStereoEffects()
{
    if (!m_effectsEnabled) return;

    if (applySpatializationEffect()) return;
}

bool SoundBaseImpl::applySpatializationEffect()
{
    // Compute the position of the sound within listener space
    auto listenerTransform = m_engine.listenerTransform();
    auto localPosition = listenerTransform * glm::vec4(m_position, 1.f);

    // Effective attenuation
    auto positionStereoOffset = std::abs(localPosition.x);
    auto alpha = 1.f - std::exp(math::EXP_HALF_LIFE * positionStereoOffset / m_spatializationHalfDistance);

    float leftAttenuation = 1.f;
    float rightAttenuation = 1.f;
    if (localPosition.x < 0.f) {
        rightAttenuation -= alpha;
    }
    else {
        leftAttenuation -= alpha;
    }

    // Apply attenuation
    for (auto i = 0u; i < m_buffer.size(); i += 2u) {
        m_buffer[i] *= leftAttenuation;
        m_buffer[i + 1] *= rightAttenuation;
    }

    return false;
}
