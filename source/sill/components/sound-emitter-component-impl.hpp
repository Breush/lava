#pragma once

#include <lava/sill/components/sound-emitter-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class SoundEmitterComponent::Impl final : public ComponentImpl {
    public:
        Impl(GameEntity& entity);

        // SoundEmitterComponent
        void add(const std::string& hrid, const std::string& path);
        void start(const std::string& hrid);

    private:
        // References
        flow::AudioEngine& m_audioEngine;

        // Resources
        std::unordered_map<std::string, std::shared_ptr<flow::SoundData>> m_soundsData;
    };
}
