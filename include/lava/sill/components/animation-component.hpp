#pragma once

#include <lava/sill/components/i-component.hpp>

#include <functional>
#include <lava/magma/material.hpp>
#include <lava/sill/animation-flags.hpp>
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
         *
         * @fixme We might want a auto-stop when animation ends.
         * Calling target afterwards would be illegal.
         */
        void start(AnimationFlags flags, float time);
        void start(AnimationFlags flags, magma::Material& material, const std::string& uniformName, float time);
        void stop(AnimationFlags flags);
        void target(AnimationFlag flag, const glm::mat4& target);
        void target(AnimationFlag flag, magma::Material& material, const std::string& uniformName, float target);
        void target(AnimationFlag flag, magma::Material& material, const std::string& uniformName, const glm::vec4& target);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
