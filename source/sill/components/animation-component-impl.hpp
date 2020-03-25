#pragma once

#include <lava/sill/components/animation-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class AnimationComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);

        // IComponent
        void update(float dt) override final;

        // AnimationComponent
        void start(AnimationFlags flags, float time);
        void start(AnimationFlags flags, magma::Material& material, const std::string& uniformName, float time);
        void stop(AnimationFlags flags);
        void target(AnimationFlag flag, const glm::mat4& target);
        void target(AnimationFlag flag, magma::Material& material, const std::string& uniformName, float target);
        void target(AnimationFlag flag, magma::Material& material, const std::string& uniformName, const glm::vec4& target);

    protected:
        struct AnimationInfo {
            bool needUpdate = true; // When timeSpent < totalTime or if target or time has changed since last update.
            float timeSpent = 0.f;
            float totalTime = 0.f;
            std::variant<glm::mat4, lava::magma::UniformFallback> startValue;
            std::variant<glm::mat4, lava::magma::UniformFallback> targetValue;

            // For AnimationFlag::MaterialUniform
            magma::Material* material;
            std::string uniformName;
            lava::magma::UniformType uniformType;
        };

    protected:
        AnimationInfo& findOrCreateAnimationInfo(AnimationFlag flag);
        AnimationInfo& findOrCreateAnimationInfo(AnimationFlag flag, magma::Material& material, const std::string& uniformName);
        void updateInterpolation(AnimationFlag flag, AnimationInfo& animationInfo);

    private:
        std::unordered_map<AnimationFlag, std::vector<AnimationInfo>> m_animationsInfos;
    };
}
