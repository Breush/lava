#pragma once

#include <lava/sill/components/i-component.hpp>

namespace lava::sill {
    class SoundEmitterComponent final : public IComponent {
    public:
        SoundEmitterComponent(Entity& entity);
        ~SoundEmitterComponent();

        // IComponent
        static std::string hrid() { return "sound-emitter"; }

        /// Sounds
        const std::unordered_map<std::string, std::string>& sounds() const { return m_sounds; }
        void add(const std::string& hrid, const std::string& path);
        void start(const std::string& hrid);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;

        std::unordered_map<std::string, std::string> m_sounds;
    };
}
