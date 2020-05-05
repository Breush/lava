#pragma once

namespace lava::dike {
    class MotionState final : public btMotionState {
    public:
        // This referenced transform will be automatically updated
        MotionState(lava::Transform& transform)
            : m_transform(transform)
        {
        }

        virtual ~MotionState() = default;

        void resetTransformChanged() { m_transformChanged = false; }
        bool transformChanged() const { return m_transformChanged; }

        void getWorldTransform(btTransform& worldTrans) const override final
        {
            btVector3 btTranslation(m_transform.translation.x, m_transform.translation.y, m_transform.translation.z);
            btQuaternion btOrientation(m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z, m_transform.rotation.w);
            worldTrans.setOrigin(btTranslation);
            worldTrans.setRotation(btOrientation);
        }

        void setWorldTransform(const btTransform& worldTrans) override final
        {
            m_transformChanged = true;
            m_transform.translation = glm::vec3{worldTrans.getOrigin().getX(), worldTrans.getOrigin().getY(), worldTrans.getOrigin().getZ()};
            m_transform.rotation = glm::quat{worldTrans.getRotation().getW(), worldTrans.getRotation().getX(), worldTrans.getRotation().getY(), worldTrans.getRotation().getZ()};
        }

    private:
        lava::Transform& m_transform;
        bool m_transformChanged = false;
    };
}
