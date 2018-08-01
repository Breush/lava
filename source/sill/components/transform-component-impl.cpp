#include "./transform-component-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace lava::sill;

TransformComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
}

void TransformComponent::Impl::translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag)
{
    // @todo Can't we write an in-place operation?
    m_transform = glm::translate(m_transform, delta);
    callPositionChanged(changeReasonFlag);
}

void TransformComponent::Impl::translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::translate(m_transform, translation - this->translation());
    callPositionChanged(changeReasonFlag);
}

void TransformComponent::Impl::onTranslationChanged(std::function<void(const glm::vec3&)> positionChangedCallback,
                                                    ChangeReasonFlags changeReasonFlags)
{
    m_positionChangedCallbacks.emplace_back(PositionChangedCallbackInfo{positionChangedCallback, changeReasonFlags});
}

//----- Internal

void TransformComponent::Impl::callPositionChanged(ChangeReasonFlag changeReasonFlag) const
{
    auto translation = glm::vec3(m_transform[3]);

    for (const auto& positionChangedCallback : m_positionChangedCallbacks) {
        if (positionChangedCallback.changeReasonFlags & changeReasonFlag) {
            positionChangedCallback.callback(translation);
        }
    }
}
