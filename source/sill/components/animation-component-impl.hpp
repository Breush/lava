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
        void stop(AnimationFlags flags);
        void target(AnimationFlag flag, const glm::mat4& target);

    protected:
        struct AnimationInfo {
            float timeSpent = 0.f;
            float totalTime = 0.f;
            std::variant<glm::mat4> startValue;
        };

    private:
        std::unordered_map<AnimationFlag, AnimationInfo> m_animationsInfos;
    };
}
