#include "./transform-component-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace lava::sill;

TransformComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
}

void TransformComponent::Impl::positionAdd(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag)
{
    // @todo Can't we write an in-place operation?
    m_transform = glm::translate(m_transform, delta);
    callPositionChanged(changeReasonFlag);
}

void TransformComponent::Impl::position(const glm::vec3& position, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::translate(m_transform, position - this->position());
    callPositionChanged(changeReasonFlag);
}

void TransformComponent::Impl::onPositionChanged(std::function<void(const glm::vec3&)> positionChangedCallback,
                                                 ChangeReasonFlags changeReasonFlags)
{
    m_positionChangedCallbacks.emplace_back(PositionChangedCallbackInfo{positionChangedCallback, changeReasonFlags});
}

//----- Internal

void TransformComponent::Impl::callPositionChanged(ChangeReasonFlag changeReasonFlag) const
{
    auto position = glm::vec3(m_transform[3]);

    for (const auto& positionChangedCallback : m_positionChangedCallbacks) {
        if (positionChangedCallback.changeReasonFlags & changeReasonFlag) {
            positionChangedCallback.callback(position);
        }
    }
}
