#include <lava/sill/components/transform-component.hpp>

#include "./transform-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(TransformComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(TransformComponent, void, update, float, dt);

// Local transform
$pimpl_method_const(TransformComponent, const glm::vec3&, translation);
$pimpl_method(TransformComponent, void, translation, const glm::vec3&, translation, ChangeReasonFlag, changeReasonFlag);
$pimpl_method(TransformComponent, void, translate, const glm::vec3&, delta, ChangeReasonFlag, changeReasonFlag);

$pimpl_method_const(TransformComponent, const glm::quat&, rotation);
$pimpl_method(TransformComponent, void, rotation, const glm::quat&, rotation, ChangeReasonFlag, changeReasonFlag);
$pimpl_method(TransformComponent, void, rotate, const glm::vec3&, axis, float, angle, ChangeReasonFlag, changeReasonFlag);

void TransformComponent::rotateAround(const glm::vec3& axis, float angle, const glm::vec3& center, ChangeReasonFlag changeReasonFlag)
{
    auto worldTransform = m_impl->worldTransform();
    worldTransform[3] -= glm::vec4(center, 0.f);
    worldTransform = glm::toMat4(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), angle, axis)) * worldTransform;
    worldTransform[3] += glm::vec4(center, 0.f);
    m_impl->worldTransform(worldTransform, changeReasonFlag);
}

$pimpl_method_const(TransformComponent, const glm::vec3&, scaling);
$pimpl_method(TransformComponent, void, scaling, const glm::vec3&, scaling, ChangeReasonFlag, changeReasonFlag);
$pimpl_method(TransformComponent, void, scale, const glm::vec3&, factors, ChangeReasonFlag, changeReasonFlag);
$pimpl_method(TransformComponent, void, scale, float, factor, ChangeReasonFlag, changeReasonFlag);

void TransformComponent::scaling(float scaling, ChangeReasonFlag changeReasonFlag)
{
    m_impl->scaling(glm::vec3{scaling, scaling, scaling}, changeReasonFlag);
}

// Local transform 2D
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

// World transform
$pimpl_method_const(TransformComponent, const glm::mat4&, worldTransform);
$pimpl_method(TransformComponent, void, worldTransform, const glm::mat4&, transform, ChangeReasonFlag, changeReasonFlag);

// Callbacks
$pimpl_method(TransformComponent, void, onTransformChanged, std::function<void()>, callback, ChangeReasonFlags,
              changeReasonFlags);
$pimpl_method(TransformComponent, void, onWorldTransformChanged, std::function<void()>, callback, ChangeReasonFlags,
              changeReasonFlags);

void TransformComponent::onTransform2dChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags)
{
    m_transform2dChangedCallbacks.emplace_back(TransformChangedCallbackInfo{callback, changeReasonFlags});
}

void TransformComponent::onWorldTransform2dChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags)
{
    m_worldTransform2dChangedCallbacks.emplace_back(TransformChangedCallbackInfo{callback, changeReasonFlags});
}

// ----- Internal

void TransformComponent::updateTransform2d(ChangeReasonFlag changeReasonFlag)
{
    m_transform2d = glm::scale(glm::mat3(1.f), m_scaling2d);
    m_transform2d = glm::rotate(glm::mat3(1.f), m_rotation2d) * m_transform2d;
    m_transform2d[2] = glm::vec3(m_translation2d, 1.f);

    callTransform2dChanged(changeReasonFlag);
}

void TransformComponent::updateWorldTransform2d(ChangeReasonFlag changeReasonFlag)
{
    if (m_entity.parent() != nullptr) {
        m_worldTransform2d = m_entity.parent()->get<TransformComponent>().worldTransform2d() * m_transform2d;
    }
    else {
        m_worldTransform2d = m_transform2d;
    }

    callWorldTransform2dChanged(changeReasonFlag);

    // Update children
    for (auto& child : m_entity.children()) {
        child->get<TransformComponent>().updateWorldTransform2d(ChangeReasonFlag::Parent);
    }
}

void TransformComponent::callTransform2dChanged(ChangeReasonFlag changeReasonFlag) const
{
    for (const auto& callback : m_transform2dChangedCallbacks) {
        if (callback.changeReasonFlags & changeReasonFlag) {
            callback.callback();
        }
    }
}

void TransformComponent::callWorldTransform2dChanged(ChangeReasonFlag changeReasonFlag) const
{
    for (const auto& callback : m_worldTransform2dChangedCallbacks) {
        if (callback.changeReasonFlags & changeReasonFlag) {
            callback.callback();
        }
    }
}
