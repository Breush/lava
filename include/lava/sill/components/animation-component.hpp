#pragma once

#include <lava/sill/components/i-component.hpp>

#include <lava/sill/animation-flags.hpp>

#include <functional>
#include <string>

namespace lava::sill {
    /**
     * Allows interpolation between two states.
     */
    class AnimationComponent final : public IComponent {
        using UpdateCallback = std::function<void(float)>;

    public:
        AnimationComponent(GameEntity& entity);
        ~AnimationComponent();

        // IComponent
        static std::string hrid() { return "animation"; }
        void update(float dt) final;

        /**
         * Controls animation over the specified flags.
         */
        void start(AnimationFlags flags, float time);
        void stop(AnimationFlags flags);
        void target(AnimationFlag flag, const glm::mat4& target);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
