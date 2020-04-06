#pragma once

#include <lava/sill/components/i-component.hpp>

#include <functional>
#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    class GameEntity;
}

namespace lava::sill {
    /**
     * Transform component.
     * The 2D and 3D transforms live independently.
     */
    // @fixme :Refactor Needs a clean-up. The pimpl is useless.
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
        ~TransformComponent();

        /// IComponent
        static std::string hrid() { return "transform"; }
        void update(float dt) override final;

        /**
         * @name Local transform
         */
        /// {
        // 3D
        const glm::vec3& translation() const;
        void translation(const glm::vec3& translation, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void translate(const glm::vec3& delta, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);

        const glm::quat& rotation() const;
        void rotation(const glm::quat& rotation, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void rotate(const glm::vec3& axis, float angle, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void rotateAround(const glm::vec3& axis, float angle, const glm::vec3& center, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);

        const glm::vec3& scaling() const;
        void scaling(const glm::vec3& scaling, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void scaling(float scaling, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void scale(const glm::vec3& factors, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);
        void scale(float factor, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);

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
         * @name World transform
         *
         * Updating the world transform will change the local transform
         * accordingly, too.
         */
        /// {
        // 3D
        const glm::mat4& worldTransform() const;
        void worldTransform(const glm::mat4& transform, ChangeReasonFlag changeReasonFlag = ChangeReasonFlag::User);

        // 2D
        const glm::mat3& worldTransform2d() const { return m_worldTransform2d; }
        /// }

        // Callbacks
        /// Called whenever the transform changed. Both transform and world transform have there correct values.
        void onTransformChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);
        /// Called whenever the world transform changed. Both transform and world transform have there correct values.
        void onWorldTransformChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);

        /// Called whenever the 2D transform changed. Both 2D transform and 2D world transform have there correct values.
        void onTransform2dChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);
        /// Called whenever the 2D world transform changed. Both 2D transform and 2D world transform have there correct values.
        void onWorldTransform2dChanged(TransformChangedCallback callback, ChangeReasonFlags changeReasonFlags = ChangeReasonFlag::All);

    protected:
        void updateTransform2d(ChangeReasonFlag changeReasonFlag);
        void updateWorldTransform2d(ChangeReasonFlag changeReasonFlag);

        void callTransform2dChanged(ChangeReasonFlag changeReasonFlag) const;
        void callWorldTransform2dChanged(ChangeReasonFlag changeReasonFlag) const;

    protected:
        struct TransformChangedCallbackInfo {
            std::function<void()> callback;
            ChangeReasonFlags changeReasonFlags;
        };

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;

        // 2D
        float m_rotation2d = 0.f;
        glm::vec2 m_translation2d = glm::vec2{0.f};
        glm::vec2 m_scaling2d = glm::vec2{1.f};
        glm::mat3 m_transform2d = glm::mat3{1.f};
        glm::mat3 m_worldTransform2d = glm::mat3{1.f};

        std::vector<TransformChangedCallbackInfo> m_transform2dChangedCallbacks;
        std::vector<TransformChangedCallbackInfo> m_worldTransform2dChangedCallbacks;
    };
}
