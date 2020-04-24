#pragma once

namespace lava::dike {
    class MotionState final : public btMotionState {
    public:
        // This referenced transform will be automatically updated
        MotionState(glm::mat4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scaling)
            : m_transform(transform)
            , m_translation(translation)
            , m_rotation(rotation)
            , m_scaling(scaling)
        {
        }

        virtual ~MotionState() = default;

        void resetTransformChanged() { m_transformChanged = false; }
        bool transformChanged() const { return m_transformChanged; }

        void getWorldTransform(btTransform& worldTrans) const override final
        {
            btVector3 btTranslation(m_translation.x, m_translation.y, m_translation.z);
            btQuaternion btOrientation(m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w);
            worldTrans.setOrigin(btTranslation);
            worldTrans.setRotation(btOrientation);
        }

        void setWorldTransform(const btTransform& worldTrans) override final
        {
            m_transformChanged = true;
            worldTrans.getOpenGLMatrix(reinterpret_cast<float*>(&m_transform));
            m_transform = glm::scale(m_transform, m_scaling);
        }

    private:
        glm::mat4& m_transform;
        glm::vec3& m_translation;
        glm::quat& m_rotation;
        glm::vec3& m_scaling;
        bool m_transformChanged = false;
    };
}
