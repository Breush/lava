#include <lava/sill/components/transform-component.hpp>

#include <lava/sill/game-entity.hpp>

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

TransformComponent::TransformComponent(GameEntity& entity)
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

void TransformComponent::rotateFrom(const glm::vec3& axis, float angle, const glm::vec3& center, ChangeReasonFlag changeReasonFlag)
{
    Transform rotationTransform;
    rotationTransform.rotation = glm::rotate(rotationTransform.rotation, angle, axis);

    m_worldTransform.translation -= center;
    m_worldTransform = rotationTransform * m_worldTransform;
    m_worldTransform.translation += center;
    worldTransform(m_worldTransform, changeReasonFlag);
}

void TransformComponent::scaling(float scaling, ChangeReasonFlag changeReasonFlag)
{
    m_transform.scaling = scaling;
    callTransformChanged(m_transformChangedCallbacks, changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
}

void TransformComponent::translation2d(const glm::vec2& translation, ChangeReasonFlag changeReasonFlag)
{
    m_translation2d = translation;
    updateTransform2d(changeReasonFlag);
    updateWorldTransform2d(changeReasonFlag);
}

void TransformComponent::rotation2d(float rotation, ChangeReasonFlag changeReasonFlag)
{
    m_rotation2d = rotation;
    updateTransform2d(changeReasonFlag);
    updateWorldTransform2d(changeReasonFlag);
}

void TransformComponent::scaling2d(const glm::vec2& scaling, ChangeReasonFlag changeReasonFlag)
{
    m_scaling2d = scaling;
    updateTransform2d(changeReasonFlag);
    updateWorldTransform2d(changeReasonFlag);
}

void TransformComponent::scaleFrom(float factor, const glm::vec3& center, ChangeReasonFlag changeReasonFlag)
{
    Transform scalingTransform;
    scalingTransform.scaling = factor;

    m_worldTransform.translation -= center;
    m_worldTransform = scalingTransform * m_worldTransform;
    m_worldTransform.translation += center;
    worldTransform(m_worldTransform, changeReasonFlag);
}

//----- Transform

void TransformComponent::transform(const lava::Transform& transform, ChangeReasonFlag changeReasonFlag)
{
    m_transform = transform;

    callTransformChanged(m_transformChangedCallbacks, changeReasonFlag);
    updateWorldTransform(changeReasonFlag);
}

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

void TransformComponent::worldTransform2d(const glm::mat3& worldTransform, ChangeReasonFlag changeReasonFlag)
{
    m_worldTransform2d = worldTransform;

    if (m_entity.parent() != nullptr) {
        m_transform2d = glm::inverse(m_entity.parent()->ensure<TransformComponent>().worldTransform2d()) * m_worldTransform2d;
    }
    else {
        m_transform2d = m_worldTransform2d;
    }

    // Update TRS
    m_translation2d.x = m_transform2d[2].x;
    m_translation2d.y = m_transform2d[2].y;
    m_rotation2d = std::atan2(m_transform2d[0].y, m_transform2d[0].x);
    m_scaling2d.x = glm::length(m_transform2d[0]);
    m_scaling2d.y = glm::length(m_transform2d[1]);

    updateChildrenWorldTransform2d();
    callTransformChanged(m_transform2dChangedCallbacks, changeReasonFlag);
    callTransformChanged(m_worldTransform2dChangedCallbacks, changeReasonFlag);
}

// ----- Callbacks

void TransformComponent::onTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags)
{
    m_transformChangedCallbacks.emplace_back(TransformChangedCallbackInfo{callback, changeReasonFlags});
}

void TransformComponent::onWorldTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags)
{
    m_worldTransformChangedCallbacks.emplace_back(TransformChangedCallbackInfo{callback, changeReasonFlags});
}

void TransformComponent::onTransform2dChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags)
{
    m_transform2dChangedCallbacks.emplace_back(TransformChangedCallbackInfo{callback, changeReasonFlags});
}

void TransformComponent::onWorldTransform2dChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags)
{
    m_worldTransform2dChangedCallbacks.emplace_back(TransformChangedCallbackInfo{callback, changeReasonFlags});
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

void TransformComponent::updateTransform2d(ChangeReasonFlag changeReasonFlag)
{
    m_transform2d = glm::scale(glm::mat3(1.f), m_scaling2d);
    m_transform2d = glm::rotate(glm::mat3(1.f), m_rotation2d) * m_transform2d;
    m_transform2d[2] = glm::vec3(m_translation2d, 1.f);

    callTransformChanged(m_transform2dChangedCallbacks, changeReasonFlag);
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
