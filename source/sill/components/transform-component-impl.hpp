#pragma once

#include <lava/sill/components/transform-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class TransformComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);

        // IComponent
        void update(float dt) final;

        // TransformComponent local transform
        const glm::vec3& translation() const { return m_translation; }
        void translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag);
        void translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag)
        {
            translation(m_translation + delta, changeReasonFlag);
        }

        const glm::quat& rotation() const { return m_rotation; }
        void rotation(const glm::quat& rotation, ChangeReasonFlag changeReasonFlag);
        void rotate(const glm::vec3& axis, float angle, ChangeReasonFlag changeReasonFlag)
        {
            rotation(glm::rotate(m_rotation, angle, axis), changeReasonFlag);
        }

        const glm::vec3& scaling() const { return m_scaling; }
        void scaling(const glm::vec3& scaling, ChangeReasonFlag changeReasonFlag);
        void scaling(float factor, ChangeReasonFlag changeReasonFlag) { scaling({factor, factor, factor}, changeReasonFlag); }
        void scale(const glm::vec3& factor, ChangeReasonFlag changeReasonFlag) { scaling(m_scaling * factor, changeReasonFlag); }
        void scale(float factor, ChangeReasonFlag changeReasonFlag) { scaling(m_scaling * factor, changeReasonFlag); }

        // TransformComponent world transform
        const glm::mat4& worldTransform() const { return m_worldTransform; } // @todo Concept of nodes/worldTransform
        void worldTransform(const glm::mat4& transform, ChangeReasonFlag changeReasonFlag);

        // TransformComponent callbacks
        void onTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags);
        void onWorldTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags);

    protected:
        void updateTransform(ChangeReasonFlag changeReasonFlag);
        void updateWorldTransform(ChangeReasonFlag changeReasonFlag);
        void callTransformChanged(ChangeReasonFlag changeReasonFlag) const;
        void callWorldTransformChanged(ChangeReasonFlag changeReasonFlag) const;

    protected:
        struct TransformChangedCallbackInfo {
            std::function<void()> callback;
            ChangeReasonFlags changeReasonFlags;
        };

    private:
        // Data
        glm::vec3 m_translation = glm::vec3(0.f);
        glm::quat m_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        glm::vec3 m_scaling = glm::vec3(1.f);
        glm::mat4 m_transform = glm::mat4(1.f);
        glm::mat4 m_worldTransform = glm::mat4(1.f);

        // Callbacks
        std::vector<TransformChangedCallbackInfo> m_transformChangedCallbacks;
        std::vector<TransformChangedCallbackInfo> m_worldTransformChangedCallbacks;
    };
}
