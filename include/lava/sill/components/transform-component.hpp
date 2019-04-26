#pragma once

#include <lava/sill/components/i-component.hpp>

#include <functional>
#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    class GameEntity;
}

namespace lava::sill {
    class TransformComponent final : public IComponent {
    public:
        enum ChangeReasonFlag : uint16_t {
            User = 0x0001,
            Physics = 0x0002,
            Parent = 0x0004,
            All = 0xFFFF,
        };

        using ChangeReasonFlags = uint16_t;

    public:
        TransformComponent(GameEntity& entity);
        ~TransformComponent();

        /// IComponent
        static std::string hrid() { return "transform"; }
        void update(float dt) override final;

        /**
         * @name Local transform
         */
        /// {
        glm::vec3 translation() const;
        void translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);

        void rotate(const glm::vec3& axis, float angle, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);

        glm::vec3 scaling() const;
        void scaling(const glm::vec3& scaling, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void scale(const glm::vec3& factors, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void scale(float factor, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        /// }

        /**
         * @name World transform
         *
         * Updating the world transform will change the local transform
         * accordingly, too.
         */
        /// {
        const glm::mat4& worldTransform() const;
        void worldTransform(const glm::mat4& transform, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        /// }

        // Callbacks
        void onTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);
        void onWorldTransformChanged(std::function<void()> callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
