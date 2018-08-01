#pragma once

#include <lava/sill/components/transform-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class TransformComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);

        // IComponent
        void update() override final {}

        // TransformComponent
        const glm::mat4& worldTransform() const { return m_transform; } // @todo Concept of nodes/worldTransform
        void translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag);
        glm::vec3 translation() const { return m_transform[3]; }
        void translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag);

        // TransformComponent callbacks
        void onTranslationChanged(std::function<void(const glm::vec3&)> positionChangedCallback,
                                  ChangeReasonFlags changeReasonFlags);

    protected:
        void callPositionChanged(ChangeReasonFlag changeReasonFlag) const;

    protected:
        struct PositionChangedCallbackInfo {
            std::function<void(const glm::vec3&)> callback;
            ChangeReasonFlags changeReasonFlags;
        };

    private:
        // Data
        glm::mat4 m_transform;

        // Callbacks
        std::vector<PositionChangedCallbackInfo> m_positionChangedCallbacks;
    };
}
