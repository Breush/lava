#pragma once

#include <lava/sill/components/i-component.hpp>

namespace lava::flow {
    class SoundData;
}

namespace lava::sill {
    class SoundEmitterComponent final : public IComponent {
    public:
        struct SoundInfo {
            std::string path;
            std::shared_ptr<flow::SoundData> soundData;
        };

    public:
        SoundEmitterComponent(Entity& entity);

        // IComponent
        static std::string hrid() { return "sound-emitter"; }

        /// Sounds
        const std::unordered_map<std::string, SoundInfo>& soundsInfos() const { return m_soundsInfos; }
        void add(const std::string& hrid, const std::string& path);
        void start(const std::string& hrid) const;

    private:
        // References
        flow::AudioEngine& m_audioEngine;

        // Resources
        std::unordered_map<std::string, SoundInfo> m_soundsInfos;
    };
}
