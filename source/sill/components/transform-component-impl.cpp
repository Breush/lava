#include "./transform-component-impl.hpp"

using namespace lava::sill;

TransformComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
}

//----- Local transform

void TransformComponent::Impl::translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::translate(m_transform, translation - this->translation());
    callTransformChanged(changeReasonFlag);
}

void TransformComponent::Impl::translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag)
{
    // @todo Can't we write in-place operations for all these transform modifications?
    m_transform = glm::translate(m_transform, delta);
    callTransformChanged(changeReasonFlag);
}

void TransformComponent::Impl::rotate(const glm::vec3& axis, float angle, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::rotate(m_transform, angle, axis);
    callTransformChanged(changeReasonFlag);
}

glm::vec3 TransformComponent::Impl::scaling() const
{
    // @todo Optimize: We could store the scaling and dirtify the value during onTransformChanged
    glm::vec3 scaling;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(m_transform, scaling, rotation, translation, skew, perspective);
    return scaling;
}

void TransformComponent::Impl::scaling(const glm::vec3& scaling, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::translate(m_transform, scaling - this->scaling());
    callTransformChanged(changeReasonFlag);
}

void TransformComponent::Impl::scale(const glm::vec3& factors, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::scale(m_transform, factors);
    callTransformChanged(changeReasonFlag);
}

void TransformComponent::Impl::scale(float factor, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::scale(m_transform, glm::vec3(factor));
    callTransformChanged(changeReasonFlag);
}

//----- Callbacks

void TransformComponent::Impl::onTransformChanged(std::function<void()> transformChangedCallback,
                                                  ChangeReasonFlags changeReasonFlags)
{
    m_transformChangedCallbacks.emplace_back(TransformChangedCallbackInfo{transformChangedCallback, changeReasonFlags});
}

//----- Internal

void TransformComponent::Impl::callTransformChanged(ChangeReasonFlag changeReasonFlag) const
{
    for (const auto& transformChangedCallback : m_transformChangedCallbacks) {
        if (transformChangedCallback.changeReasonFlags & changeReasonFlag) {
            transformChangedCallback.callback();
        }
    }
}
