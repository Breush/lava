#pragma once

#include <lava/sill/components/i-component.hpp>

#include <functional>
#include <glm/glm.hpp>
#include <lava/core/transform.hpp>
#include <string>

namespace lava::sill {
    class GameEntity;
}

namespace lava::sill {
    /**
     * Transform component.
     * The 2D and 3D transforms live independently.
     */
    class TransformComponent final : public IComponent {
    public:
        using TransformChangedCallback = std::function<void()>;

        enum ChangeReasonFlag : uint16_t {
            User = 0x0001,
            Physics = 0x0002,
            Parent = 0x0004,
            Animation = 0x0008,
            All = 0xFFFF,
        };

        using ChangeReasonFlags = uint16_t;

    public:
        TransformComponent(GameEntity& entity);

        /// IComponent
        static std::string hrid() { return "transform"; }
        void update(float dt) final;

        /**
         * @name Local transform
         */
        /// {
        // 3D
        const glm::vec3& translation() const { return m_transform.translation; }
        void translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User) {
            translation(m_transform.translation + delta, changeReasonFlag);
        }

        const glm::quat& rotation() const { return m_transform.rotation; }
        void rotation(const glm::quat& rotation, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void rotate(const glm::vec3& axis, float angle, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User) {
            rotation(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), angle, axis) * m_transform.rotation, changeReasonFlag);
        }
        void rotateAround(const glm::vec3& axis, float angle, const glm::vec3& center, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);

        float scaling() const { return m_transform.scaling; }
        void scaling(float scaling, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void scale(float factor, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User) {
            scaling(m_transform.scaling * factor, changeReasonFlag);
        }

        // 2D
        const glm::vec2& translation2d() const { return m_translation2d; }
        void translation2d(const glm::vec2& translation2d, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void translate2d(const glm::vec2& delta, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User) {
            translation2d(m_translation2d + delta, changeReasonFlag);
        }

        float rotation2d() const { return m_rotation2d; }
        void rotation2d(float rotation, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void rotate2d(float angleDelta, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User) {
            rotation2d(m_rotation2d + angleDelta, changeReasonFlag);
        }

        const glm::vec2& scaling2d() const { return m_scaling2d; }
        void scaling2d(const glm::vec2& scaling, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void scaling2d(float scaling, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User) {
            scaling2d({scaling, scaling}, changeReasonFlag);
        }
        void scale2d(const glm::vec2& factors, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User) {
            scaling2d(factors * m_scaling2d, changeReasonFlag);
        }
        void scale2d(float factor, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User) {
            scaling2d(factor * m_scaling2d, changeReasonFlag);
        }
        /// }

        /**
         * @name Transform
         *
         * Updating the world transform will change the local transform
         * accordingly, too.
         */
        /// {
        // 3D
        const lava::Transform& transform() const { return m_transform; }
        void transform(const lava::Transform& transform, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);

        const lava::Transform& worldTransform() const { return m_worldTransform; }
        void worldTransform(const lava::Transform& worldTransform, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);

        // 2D
        // @todo :Terminology Well, have lava::Transform2d!
        // But be careful with non-uniform scaling that might be more used
        // than in 3D.
        const glm::mat3& worldTransform2d() const { return m_worldTransform2d; }
        void worldTransform2d(const glm::mat3& worldTransform, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        /// }

        /**
         * @name Callbacks
         */
        /// {
        /// Called whenever the transform changed. Both transform and world transform have there correct values.
        void onTransformChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);
        /// Called whenever the world transform changed. Both transform and world transform have there correct values.
        void onWorldTransformChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);

        /// Called whenever the 2D transform changed. Both 2D transform and 2D world transform have there correct values.
        void onTransform2dChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);
        /// Called whenever the 2D world transform changed. Both 2D transform and 2D world transform have there correct values.
        void onWorldTransform2dChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);
        /// }

    protected:
        void updateTransform(ChangeReasonFlag changeReasonFlag);
        void updateWorldTransform(ChangeReasonFlag changeReasonFlag);
        void updateChildrenWorldTransform();
        void updateTransform2d(ChangeReasonFlag changeReasonFlag);
        void updateWorldTransform2d(ChangeReasonFlag changeReasonFlag);
        void updateChildrenWorldTransform2d();

    public:
        struct TransformChangedCallbackInfo {
            std::function<void()> callback;
            ChangeReasonFlags changeReasonFlags;
        };

    private:
        // 3D
        lava::Transform m_transform;
        lava::Transform m_worldTransform;

        // 2D
        float m_rotation2d = 0.f;
        glm::vec2 m_translation2d = glm::vec2{0.f};
        glm::vec2 m_scaling2d = glm::vec2{1.f};
        glm::mat3 m_transform2d = glm::mat3{1.f};
        glm::mat3 m_worldTransform2d = glm::mat3{1.f};

        // Callbacks
        std::vector<TransformChangedCallbackInfo> m_transformChangedCallbacks;
        std::vector<TransformChangedCallbackInfo> m_worldTransformChangedCallbacks;
        std::vector<TransformChangedCallbackInfo> m_transform2dChangedCallbacks;
        std::vector<TransformChangedCallbackInfo> m_worldTransform2dChangedCallbacks;

        bool m_everUpdated = false;
    };
}
