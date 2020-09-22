#include <lava/sill/components/transform-component.hpp>

#include <lava/sill/entity.hpp>

using namespace lava::sill;

namespace {
    void callTransformChanged(const std::vector<TransformComponent::TransformChangedCallbackInfo>& callbacks,
                              TransformComponent::ChangeReasonFlags changeReasonFlags)
    {
        for (const auto& callback : callbacks) {
            if (callback.changeReasonFlags & changeReasonFlags) {
                callback.callback();
            }
        }
    }
}

TransformComponent::TransformComponent(Entity& entity)
    : IComponent(entity)
{
    entity.onParentChanged([this] {
        // @note :EverUpdated We want to manager two situations here.
        // - When parenting/unparenting we want to never change
        //   the world position so that it looks like a no-op.
        // - When creating a hierarchy of entities where the root one
        //   has already been placed, but the parenting occurs afterwards,
        //   we want the init behavior we have when constructing.

        if (m_everUpdated) {
            // Keep the entity at the same world space position.
            worldTransform(m_worldTransform, ChangeReasonFlag::Parent);
            worldTransform2d(m_worldTransform2d, ChangeReasonFlag::Parent);
        } else {
            // Move the entity to the parent space.
            updateWorldTransform(ChangeReasonFlag::Parent);
            updateWorldTransform2d(ChangeReasonFlag::Parent);
        }
    });

    // Place the entity according to parent
    if (entity.parent() != nullptr) {
        updateWorldTransform(ChangeReasonFlag::Parent);
        updateWorldTransform2d(ChangeReasonFlag::Parent);
    }
}

void TransformComponent::update(float /* dt */)
{
    // @note :EverUpdated
    m_everUpdated = true;
}

//----- Local transform

void TransformComponent::transform(const lava::Transform& transform, ChangeReasonFlag changeReasonFlag)
{
    m_transform = transform;

    callTransformChanged(m_transformChangedCallbacks, changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
}

void TransformComponent::translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag)
{
    m_transform.translation = translation;
    callTransformChanged(m_transformChangedCallbacks, changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
}

void TransformComponent::rotation(const glm::quat& rotation, ChangeReasonFlag changeReasonFlag)
{
    m_transform.rotation = rotation;
    callTransformChanged(m_transformChangedCallbacks, changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
}

void TransformComponent::scaling(float scaling, ChangeReasonFlag changeReasonFlag)
{
    m_transform.scaling = scaling;
    callTransformChanged(m_transformChangedCallbacks, changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
}

void TransformComponent::translation2d(const glm::vec2& translation, ChangeReasonFlag changeReasonFlag)
{
    m_transform2d.translation = translation;
    callTransformChanged(m_transform2dChangedCallbacks, changeReasonFlag);
    updateWorldTransform2d(changeReasonFlag);
}

void TransformComponent::rotation2d(float rotation, ChangeReasonFlag changeReasonFlag)
{
    m_transform2d.rotation = rotation;
    callTransformChanged(m_transform2dChangedCallbacks, changeReasonFlag);
    updateWorldTransform2d(changeReasonFlag);
}

void TransformComponent::scaling2d(float scaling, ChangeReasonFlag changeReasonFlag)
{
    m_transform2d.scaling = scaling;
    callTransformChanged(m_transform2dChangedCallbacks, changeReasonFlag);
    updateWorldTransform2d(changeReasonFlag);
}

//----- World transform

void TransformComponent::worldTransform(const lava::Transform& worldTransform, ChangeReasonFlag changeReasonFlag)
{
    m_worldTransform = worldTransform;

    if (m_entity.parent() != nullptr) {
        m_transform = m_entity.parent()->ensure<TransformComponent>().worldTransform().inverse() * m_worldTransform;
    }
    else {
        m_transform = m_worldTransform;
    }

    updateChildrenWorldTransform();
    callTransformChanged(m_transformChangedCallbacks, changeReasonFlag);
    callTransformChanged(m_worldTransformChangedCallbacks, changeReasonFlag);
}

void TransformComponent::worldTranslate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag)
{
    m_worldTransform.translation += delta;
    worldTransform(m_worldTransform, changeReasonFlag);
}

void TransformComponent::worldRotateFrom(const glm::vec3& axis, float angle, const glm::vec3& center, ChangeReasonFlag changeReasonFlag)
{
    Transform rotationTransform;
    rotationTransform.rotation = glm::rotate(rotationTransform.rotation, angle, axis);

    m_worldTransform.translation -= center;
    m_worldTransform = rotationTransform * m_worldTransform;
    m_worldTransform.translation += center;
    worldTransform(m_worldTransform, changeReasonFlag);
}

void TransformComponent::worldScaleFrom(float factor, const glm::vec3& center, ChangeReasonFlag changeReasonFlag)
{
    Transform scalingTransform;
    scalingTransform.scaling = factor;

    m_worldTransform.translation -= center;
    m_worldTransform = scalingTransform * m_worldTransform;
    m_worldTransform.translation += center;
    worldTransform(m_worldTransform, changeReasonFlag);
}

void TransformComponent::worldTransform2d(const lava::Transform2d& worldTransform, ChangeReasonFlag changeReasonFlag)
{
    m_worldTransform2d = worldTransform;

    if (m_entity.parent() != nullptr) {
        m_transform2d = m_entity.parent()->ensure<TransformComponent>().worldTransform2d().inverse() * m_worldTransform2d;
    }
    else {
        m_transform2d = m_worldTransform2d;
    }

    updateChildrenWorldTransform2d();
    callTransformChanged(m_transform2dChangedCallbacks, changeReasonFlag);
    callTransformChanged(m_worldTransform2dChangedCallbacks, changeReasonFlag);
}

// ----- Callbacks

void TransformComponent::onTransformChanged(TransformChangedCallback&& callback, ChangeReasonFlags changeReasonFlags)
{
    m_transformChangedCallbacks.emplace_back(TransformChangedCallbackInfo{std::move(callback), changeReasonFlags});
}

void TransformComponent::onWorldTransformChanged(TransformChangedCallback&& callback, ChangeReasonFlags changeReasonFlags)
{
    m_worldTransformChangedCallbacks.emplace_back(TransformChangedCallbackInfo{std::move(callback), changeReasonFlags});
}

void TransformComponent::onTransform2dChanged(TransformChangedCallback&& callback, ChangeReasonFlags changeReasonFlags)
{
    m_transform2dChangedCallbacks.emplace_back(TransformChangedCallbackInfo{std::move(callback), changeReasonFlags});
}

void TransformComponent::onWorldTransform2dChanged(TransformChangedCallback&& callback, ChangeReasonFlags changeReasonFlags)
{
    m_worldTransform2dChangedCallbacks.emplace_back(TransformChangedCallbackInfo{std::move(callback), changeReasonFlags});
}

// ----- Internal

void TransformComponent::updateWorldTransform(ChangeReasonFlag changeReasonFlag)
{
    if (m_entity.parent() != nullptr) {
        m_worldTransform = m_entity.parent()->ensure<TransformComponent>().worldTransform() * m_transform;
    }
    else {
        m_worldTransform = m_transform;
    }

    updateChildrenWorldTransform();
    callTransformChanged(m_worldTransformChangedCallbacks, changeReasonFlag);
}

void TransformComponent::updateChildrenWorldTransform()
{
    for (auto& child : m_entity.children()) {
        child->ensure<TransformComponent>().updateWorldTransform(ChangeReasonFlag::Parent);
    }
}

void TransformComponent::updateWorldTransform2d(ChangeReasonFlag changeReasonFlag)
{
    if (m_entity.parent() != nullptr) {
        m_worldTransform2d = m_entity.parent()->ensure<TransformComponent>().worldTransform2d() * m_transform2d;
    }
    else {
        m_worldTransform2d = m_transform2d;
    }

    updateChildrenWorldTransform2d();
    callTransformChanged(m_worldTransform2dChangedCallbacks, changeReasonFlag);
}

void TransformComponent::updateChildrenWorldTransform2d()
{
    for (auto& child : m_entity.children()) {
        child->ensure<TransformComponent>().updateWorldTransform2d(ChangeReasonFlag::Parent);
    }
}
