#pragma once

#include <lava/sill/components/transform-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class TransformComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);

        // IComponent
        void update() override final {}

        // TransformComponent local transform
        glm::vec3 translation() const { return m_transform[3]; }
        void translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag);
        void translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag);

        glm::vec3 scaling() const;
        void scaling(const glm::vec3& scaling, ChangeReasonFlag changeReasonFlag);
        void scale(const glm::vec3& factors, ChangeReasonFlag changeReasonFlag);
        void scale(float factor, ChangeReasonFlag changeReasonFlag);

        // TransformComponent world transform
        const glm::mat4& worldTransform() const { return m_transform; } // @todo Concept of nodes/worldTransform

        // TransformComponent callbacks
        void onTransformChanged(std::function<void()> transformChangedCallback, ChangeReasonFlags changeReasonFlags);

    protected:
        void callTransformChanged(ChangeReasonFlag changeReasonFlag) const;

    protected:
        struct TransformChangedCallbackInfo {
            std::function<void()> callback;
            ChangeReasonFlags changeReasonFlags;
        };

    private:
        // Data
        glm::mat4 m_transform;

        // Callbacks
        std::vector<TransformChangedCallbackInfo> m_transformChangedCallbacks;
    };
}
