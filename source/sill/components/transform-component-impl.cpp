#include "./transform-component-impl.hpp"

using namespace lava::sill;

TransformComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
}

void TransformComponent::Impl::update(float /*dt*/) {}

//----- Local transform

void TransformComponent::Impl::translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag)
{
    m_translation = translation;

    updateTransform(changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
}

void TransformComponent::Impl::rotation(const glm::quat& rotation, ChangeReasonFlag changeReasonFlag)
{
    m_rotation = rotation;

    updateTransform(changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
}

void TransformComponent::Impl::scaling(const glm::vec3& scaling, ChangeReasonFlag changeReasonFlag)
{
    m_scaling = scaling;

    updateTransform(changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
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

    // Update TRS
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(m_transform, m_scaling, m_rotation, m_translation, skew, perspective);

    callTransformChanged(changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
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

void TransformComponent::Impl::updateTransform(ChangeReasonFlag changeReasonFlag)
{
    m_transform = glm::scale(glm::mat4(1.f), m_scaling);
    m_transform = glm::mat4(m_rotation) * m_transform;
    m_transform[3] = glm::vec4(m_translation, 1.f);

    callTransformChanged(changeReasonFlag);
}

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
