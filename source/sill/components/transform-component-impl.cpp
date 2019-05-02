#include "./transform-component-impl.hpp"

using namespace lava::sill;

TransformComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
}

void TransformComponent::Impl::update(float /*dt*/)
{
}

//----- Local transform

void TransformComponent::Impl::translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag)
{
    m_transform[3] = glm::vec4(translation, 1.f);
    updateWorldTransform(changeReasonFlag);
    callTransformChanged(changeReasonFlag);
}

void TransformComponent::Impl::translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag)
{
    // @todo Can't we write in-place operations for all these transform modifications?
    m_transform = glm::translate(m_transform, delta);
    updateWorldTransform(changeReasonFlag);
    callTransformChanged(changeReasonFlag);
}

void TransformComponent::Impl::rotate(const glm::vec3& axis, float angle, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::rotate(m_transform, angle, axis);
    updateWorldTransform(changeReasonFlag);
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
    // @fixme That probably doesn't work... Thanks copy-paste!
    m_transform = glm::translate(m_transform, scaling - this->scaling());
    updateWorldTransform(changeReasonFlag);
    callTransformChanged(changeReasonFlag);
}

void TransformComponent::Impl::scale(const glm::vec3& factors, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::scale(m_transform, factors);
    updateWorldTransform(changeReasonFlag);
    callTransformChanged(changeReasonFlag);
}

void TransformComponent::Impl::scale(float factor, ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::scale(m_transform, glm::vec3(factor));
    updateWorldTransform(changeReasonFlag);
    callTransformChanged(changeReasonFlag);
}

//----- World transform

void TransformComponent::Impl::worldTransform(const glm::mat4& transform, ChangeReasonFlag changeReasonFlag)
{
    m_worldTransform = transform;

    if (m_entity.parent() != nullptr) {
        m_transform = glm::inverse(m_entity.parent()->get<TransformComponent>().worldTransform()) * m_worldTransform;
    }
    else {
        m_transform = m_worldTransform;
    }

    updateWorldTransform(changeReasonFlag);
    callTransformChanged(changeReasonFlag);
}

//----- Callbacks

void TransformComponent::Impl::onTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags)
{
    m_transformChangedCallbacks.emplace_back(TransformChangedCallbackInfo{callback, changeReasonFlags});
}

void TransformComponent::Impl::onWorldTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags)
{
    m_worldTransformChangedCallbacks.emplace_back(TransformChangedCallbackInfo{callback, changeReasonFlags});
}

//----- Internal

void TransformComponent::Impl::updateWorldTransform(ChangeReasonFlag changeReasonFlag)
{
    if (m_entity.parent() != nullptr) {
        m_worldTransform = m_entity.parent()->get<TransformComponent>().worldTransform() * m_transform;
    }
    else {
        m_worldTransform = m_transform;
    }

    callWorldTransformChanged(changeReasonFlag);

    // Update children
    for (auto& child : m_entity.children()) {
        child->get<TransformComponent>().impl().updateWorldTransform(ChangeReasonFlag::Parent);
    }
}

void TransformComponent::Impl::callTransformChanged(ChangeReasonFlag changeReasonFlag) const
{
    for (const auto& callback : m_transformChangedCallbacks) {
        if (callback.changeReasonFlags & changeReasonFlag) {
            callback.callback();
        }
    }
}

void TransformComponent::Impl::callWorldTransformChanged(ChangeReasonFlag changeReasonFlag) const
{
    for (const auto& callback : m_worldTransformChangedCallbacks) {
        if (callback.changeReasonFlags & changeReasonFlag) {
            callback.callback();
        }
    }
}
