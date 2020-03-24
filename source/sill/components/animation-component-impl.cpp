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

    for (auto& iAnimationInfos : m_animationsInfos) {
        auto& animationInfos = iAnimationInfos.second;
        for (auto& animationInfo : animationInfos) {
            animationInfo.timeSpent += dt;
            if (animationInfo.needUpdate) {
                updateInterpolation(iAnimationInfos.first, animationInfo);
            }
        }
    }
}

// AnimationComponent
void AnimationComponent::Impl::start(AnimationFlags flags, float time)
{
    if (flags & AnimationFlag::WorldTransform) {
        auto& animationInfo = findOrCreateAnimationInfo(AnimationFlag::WorldTransform);
        animationInfo.timeSpent = 0.f;
        animationInfo.totalTime = time;
        animationInfo.startValue = m_entity.get<TransformComponent>().worldTransform();
        animationInfo.targetValue = animationInfo.startValue;
        animationInfo.needUpdate = true;
    }
}
void AnimationComponent::Impl::start(AnimationFlags flags, magma::Material& material, const std::string& uniformName, float time)
{
    if (flags & AnimationFlag::MaterialUniform) {
        auto& animationInfo = findOrCreateAnimationInfo(AnimationFlag::MaterialUniform, material, uniformName);
        animationInfo.timeSpent = 0.f;
        animationInfo.totalTime = time;
        animationInfo.startValue = material.get_vec4(uniformName);
        animationInfo.targetValue = animationInfo.startValue;
        animationInfo.needUpdate = true;
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
    auto& animationInfo = findOrCreateAnimationInfo(flag);
    animationInfo.targetValue = target;
    animationInfo.needUpdate = true;
}

void AnimationComponent::Impl::target(AnimationFlag flag, magma::Material& material, const std::string& uniformName,
                                      const glm::vec4& target)
{
    auto& animationInfo = findOrCreateAnimationInfo(flag, material, uniformName);
    animationInfo.targetValue = target;
    animationInfo.uniformType = UniformType::Vec4;
    animationInfo.needUpdate = true;
}

// Internal

AnimationComponent::Impl::AnimationInfo& AnimationComponent::Impl::findOrCreateAnimationInfo(AnimationFlag flag)
{
    if (flag == AnimationFlag::WorldTransform) {
        auto& animationInfos = m_animationsInfos[AnimationFlag::WorldTransform];
        animationInfos.resize(1);
        return animationInfos[0];
    }

    return m_animationsInfos[AnimationFlag::None][0];
}

AnimationComponent::Impl::AnimationInfo& AnimationComponent::Impl::findOrCreateAnimationInfo(AnimationFlag flag,
                                                                                             magma::Material& material,
                                                                                             const std::string& uniformName)
{
    auto& animationInfos = m_animationsInfos[flag];
    for (auto& animationInfo : animationInfos) {
        if (animationInfo.material == &material && animationInfo.uniformName == uniformName) {
            return animationInfo;
        }
    }

    animationInfos.emplace_back();
    animationInfos.back().material = &material;
    animationInfos.back().uniformName = uniformName;
    return animationInfos.back();
}

void AnimationComponent::Impl::updateInterpolation(AnimationFlag flag, AnimationInfo& animationInfo)
{
    float timeRatio = animationInfo.timeSpent / animationInfo.totalTime;

    // We need to update next loop, because animation is not over yet.
    animationInfo.needUpdate = (timeRatio < 1.f);

    if (flag == AnimationFlag::WorldTransform) {
        const auto& startValue = std::get<glm::mat4>(animationInfo.startValue);
        const auto& targetValue = std::get<glm::mat4>(animationInfo.targetValue);

        glm::mat4 value = targetValue;
        if (animationInfo.needUpdate) value = chamber::interpolateLinear(startValue, targetValue, timeRatio);
        m_entity.get<TransformComponent>().worldTransform(value, TransformComponent::ChangeReasonFlag::Animation);
    }
    else if (flag == AnimationFlag::MaterialUniform) {
        if (animationInfo.uniformType == UniformType::Vec4) {
            const auto& startValue = std::get<glm::vec4>(animationInfo.startValue);
            const auto& targetValue = std::get<glm::vec4>(animationInfo.targetValue);

            glm::vec4 value = targetValue;
            if (animationInfo.needUpdate) value = chamber::interpolateLinear(startValue, targetValue, timeRatio);
            animationInfo.material->set(animationInfo.uniformName, value);
        }
    }
}
