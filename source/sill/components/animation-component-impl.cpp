#include "./animation-component-impl.hpp"

#include <lava/sill/components.hpp>

using namespace lava::chamber;
using namespace lava::sill;

AnimationComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
}

// IComponent
void AnimationComponent::Impl::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    for (auto& iAnimationInfo : m_animationsInfos) {
        auto& animationInfo = iAnimationInfo.second;
        animationInfo.timeSpent += dt;
    }
}

// AnimationComponent
void AnimationComponent::Impl::start(AnimationFlags flags, float time)
{
    if (flags & AnimationFlag::WorldTransform) {
        auto& animationInfo = m_animationsInfos[AnimationFlag::WorldTransform];
        animationInfo.timeSpent = 0.f;
        animationInfo.totalTime = time;
        animationInfo.startValue = m_entity.get<TransformComponent>().worldTransform();
    }
}

void AnimationComponent::Impl::stop(AnimationFlags flags)
{
    if (flags & AnimationFlag::WorldTransform) {
        m_animationsInfos.erase(AnimationFlag::WorldTransform);
    }
}

void AnimationComponent::Impl::target(AnimationFlag flag, const glm::mat4& target)
{
    auto& animationInfo = m_animationsInfos[flag];
    const auto& startValue = std::get<glm::mat4>(animationInfo.startValue);

    glm::mat4 value = target;
    if (animationInfo.timeSpent < animationInfo.totalTime) {
        value = glm::interpolate(startValue, target, animationInfo.timeSpent / animationInfo.totalTime);
    }

    if (flag == AnimationFlag::WorldTransform) {
        m_entity.get<TransformComponent>().worldTransform(value);
    }
    else {
        logger.warning("sill.animation-component") << "Target glm::mat4 is not valid for specified flag." << std::endl;
    }
}
