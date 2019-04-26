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
        glm::vec3 translation() const { return m_transform[3]; }
        void translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag);
        void translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag);

        void rotate(const glm::vec3& axis, float angle, ChangeReasonFlag changeReasonFlag);

        glm::vec3 scaling() const;
        void scaling(const glm::vec3& scaling, ChangeReasonFlag changeReasonFlag);
        void scale(const glm::vec3& factors, ChangeReasonFlag changeReasonFlag);
        void scale(float factor, ChangeReasonFlag changeReasonFlag);

        // TransformComponent world transform
        const glm::mat4& worldTransform() const { return m_worldTransform; } // @todo Concept of nodes/worldTransform
        void worldTransform(const glm::mat4& transform, ChangeReasonFlag changeReasonFlag);

        // TransformComponent callbacks
        void onTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags);
        void onWorldTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags);

    protected:
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
        glm::mat4 m_transform = glm::mat4(1.f);
        glm::mat4 m_worldTransform = glm::mat4(1.f);

        // Callbacks
        std::vector<TransformChangedCallbackInfo> m_transformChangedCallbacks;
        std::vector<TransformChangedCallbackInfo> m_worldTransformChangedCallbacks;
    };
}
