#pragma once

#include <lava/sill/components/i-component.hpp>

namespace lava::sill {
    class SoundEmitterComponent final : public IComponent {
    public:
        SoundEmitterComponent(GameEntity& entity);
        ~SoundEmitterComponent();

        // IComponent
        static std::string hrid() { return "sound-emitter"; }
        void update(float dt) final;

        /// Sounds
        void add(const std::string& hrid, const std::string& path);
        void start(const std::string& hrid);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
