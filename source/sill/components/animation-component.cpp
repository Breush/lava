#include <lava/sill/components/animation-component.hpp>

#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/entity.hpp>

using namespace lava::chamber;
using namespace lava::sill;

AnimationComponent::AnimationComponent(Entity& entity)
    : IComponent(entity)
{
}

void AnimationComponent::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    for (auto& iAnimationInfos : m_animationsInfos) {
        auto& animationInfos = iAnimationInfos.second;
        for (auto iAnimationInfo = animationInfos.rbegin(); iAnimationInfo != animationInfos.rend(); ++iAnimationInfo) {
            auto& animationInfo = *iAnimationInfo;
            animationInfo.timeSpent += dt;

            if (animationInfo.needUpdate) {
                updateInterpolation(iAnimationInfos.first, animationInfo);

                // Remove all animations that need to be removed.
                if (animationInfo.autoStop && !animationInfo.needUpdate) {
                    animationInfos.erase(std::next(iAnimationInfo).base());
                }
            }
        }
    }
}

// ----- Control

void AnimationComponent::start(AnimationFlags flags, float time, bool autoStop)
{
    if (flags & AnimationFlag::Transform) {
        auto& animationInfo = findOrCreateAnimationInfo(AnimationFlag::Transform);
        animationInfo.autoStop = autoStop;
        animationInfo.timeSpent = 0.f;
        animationInfo.totalTime = time;
        animationInfo.startValue = m_entity.get<TransformComponent>().transform();
        animationInfo.targetValue = animationInfo.startValue;
        animationInfo.needUpdate = true;
    }
    else if (flags & AnimationFlag::WorldTransform) {
        auto& animationInfo = findOrCreateAnimationInfo(AnimationFlag::WorldTransform);
        animationInfo.autoStop = autoStop;
        animationInfo.timeSpent = 0.f;
        animationInfo.totalTime = time;
        animationInfo.startValue = m_entity.get<TransformComponent>().worldTransform();
        animationInfo.targetValue = animationInfo.startValue;
        animationInfo.needUpdate = true;
    }
}
void AnimationComponent::start(AnimationFlags flags, magma::Material& material, const std::string& uniformName, float time, bool autoStop)
{
    if (flags & AnimationFlag::MaterialUniform) {
        const auto& attribute = material.attributes().at(uniformName);
        auto& animationInfo = findOrCreateAnimationInfo(AnimationFlag::MaterialUniform, material, uniformName);
        animationInfo.autoStop = autoStop;
        animationInfo.timeSpent = 0.f;
        animationInfo.totalTime = time;
        animationInfo.startValue = attribute.value;
        animationInfo.uniformType = attribute.type;
        animationInfo.targetValue = animationInfo.startValue;
        animationInfo.needUpdate = true;
    }
}

void AnimationComponent::stop(AnimationFlags flags)
{
    if (flags & AnimationFlag::Transform) {
        m_animationsInfos.erase(AnimationFlag::Transform);
    }
    else if (flags & AnimationFlag::WorldTransform) {
        m_animationsInfos.erase(AnimationFlag::WorldTransform);
    }
}

void AnimationComponent::target(AnimationFlag flag, const lava::Transform& target)
{
    auto animationInfo = findAnimationInfo(flag);
    if (animationInfo == nullptr) {
        logger.error("sill.components.animation") << "Targeting an non-existing animation." << std::endl;
    }

    animationInfo->targetValue = target;
    animationInfo->needUpdate = true;
}

void AnimationComponent::target(AnimationFlag flag, magma::Material& material, const std::string& uniformName,
                                      const glm::vec4& target)
{
    auto animationInfo = findAnimationInfo(flag, material, uniformName);
    if (animationInfo == nullptr) {
        logger.error("sill.components.animation") << "Targeting an non-existing animation." << std::endl;
    }

    animationInfo->targetValue = target;
    animationInfo->needUpdate = true;
}

void AnimationComponent::target(AnimationFlag flag, magma::Material& material, const std::string& uniformName,
                                      float target)
{
    auto& animationInfo = findOrCreateAnimationInfo(flag, material, uniformName);
    animationInfo.targetValue = target;
    animationInfo.needUpdate = true;
}

// ----- Internal

AnimationComponent::AnimationInfo* AnimationComponent::findAnimationInfo(AnimationFlag flag)
{
    auto animationInfosIt = m_animationsInfos.find(flag);
    if (animationInfosIt == m_animationsInfos.end()) {
        return nullptr;
    }

    if (flag == AnimationFlag::Transform) {
        if (animationInfosIt->second.size() == 0) {
            return nullptr;
        }
        return &animationInfosIt->second.at(0);
    }
    else if (flag == AnimationFlag::WorldTransform) {
        if (animationInfosIt->second.size() == 0) {
            return nullptr;
        }
        return &animationInfosIt->second.at(0);
    }

    return nullptr;
}

AnimationComponent::AnimationInfo* AnimationComponent::findAnimationInfo(AnimationFlag flag,
                                                                         magma::Material& material,
                                                                         const std::string& uniformName)
{
    auto animationInfosIt = m_animationsInfos.find(flag);
    if (animationInfosIt == m_animationsInfos.end()) {
        return nullptr;
    }

    for (auto& animationInfo : animationInfosIt->second) {
        if (animationInfo.material == &material && animationInfo.uniformName == uniformName) {
            return &animationInfo;
        }
    }

    return nullptr;
}

AnimationComponent::AnimationInfo& AnimationComponent::findOrCreateAnimationInfo(AnimationFlag flag)
{
    if (flag == AnimationFlag::Transform) {
        auto& animationInfos = m_animationsInfos[AnimationFlag::Transform];
        animationInfos.resize(1);
        return animationInfos[0];
    }
    else if (flag == AnimationFlag::WorldTransform) {
        auto& animationInfos = m_animationsInfos[AnimationFlag::WorldTransform];
        animationInfos.resize(1);
        return animationInfos[0];
    }

    // Should not happen.
    return m_animationsInfos[AnimationFlag::None][0];
}

AnimationComponent::AnimationInfo& AnimationComponent::findOrCreateAnimationInfo(AnimationFlag flag,
                                                                                 magma::Material& material,
                                                                                 const std::string& uniformName)
{
    auto pAnimationInfo = findAnimationInfo(flag, material, uniformName);
    if (pAnimationInfo != nullptr) {
        return *pAnimationInfo;
    }

    auto& animationInfos = m_animationsInfos[flag];
    auto& animationInfo = animationInfos.emplace_back();
    animationInfo.material = &material;
    animationInfo.uniformName = uniformName;
    return animationInfo;
}

void AnimationComponent::updateInterpolation(AnimationFlag flag, AnimationInfo& animationInfo)
{
    float timeRatio = animationInfo.timeSpent / animationInfo.totalTime;

    // We need to update next loop, because animation is not over yet.
    animationInfo.needUpdate = (timeRatio < 1.f);

    if (flag == AnimationFlag::Transform) {
        const auto& startValue = std::get<lava::Transform>(animationInfo.startValue);
        const auto& targetValue = std::get<lava::Transform>(animationInfo.targetValue);

        lava::Transform value = targetValue;
        if (animationInfo.needUpdate) value = chamber::interpolate(startValue, targetValue, timeRatio, InterpolationEase::In, InterpolationEase::None, InterpolationEase::In);
        m_entity.get<TransformComponent>().transform(value, TransformComponent::ChangeReasonFlag::Animation);
    }
    else if (flag == AnimationFlag::WorldTransform) {
        const auto& startValue = std::get<lava::Transform>(animationInfo.startValue);
        const auto& targetValue = std::get<lava::Transform>(animationInfo.targetValue);

        lava::Transform value = targetValue;
        if (animationInfo.needUpdate) value = chamber::interpolate(startValue, targetValue, timeRatio, InterpolationEase::In, InterpolationEase::None, InterpolationEase::In);
        m_entity.get<TransformComponent>().worldTransform(value, TransformComponent::ChangeReasonFlag::Animation);
    }
    else if (flag == AnimationFlag::MaterialUniform) {
        if (animationInfo.uniformType == magma::UniformType::Float) {
            auto startValue = std::get<magma::UniformFallback>(animationInfo.startValue).floatValue;
            auto targetValue = std::get<magma::UniformFallback>(animationInfo.targetValue).floatValue;

            auto value = targetValue;
            if (animationInfo.needUpdate) value = chamber::interpolateLinear(startValue, targetValue, timeRatio);
            animationInfo.material->set(animationInfo.uniformName, value);
        }
        else if (animationInfo.uniformType == magma::UniformType::Vec4) {
            const auto& startValue = std::get<magma::UniformFallback>(animationInfo.startValue).vec4Value;
            const auto& targetValue = std::get<magma::UniformFallback>(animationInfo.targetValue).vec4Value;

            auto value = targetValue;
            if (animationInfo.needUpdate) value = chamber::interpolateLinear(startValue, targetValue, timeRatio);
            animationInfo.material->set(animationInfo.uniformName, value);
        }
    }
}
