#pragma once

namespace lava::dike {
    class MotionState final : public btMotionState {
    public:
        // This referenced transform will be automatically updated
        MotionState(glm::mat4& transform)
            : m_transform(transform)
        {
        }

        virtual ~MotionState() = default;

        void resetTransformChanged() { m_transformChanged = false; }
        bool transformChanged() const { return m_transformChanged; }

        void getWorldTransform(btTransform& worldTrans) const override final
        {
            worldTrans.setFromOpenGLMatrix(reinterpret_cast<const float*>(&m_transform));
        }

        void setWorldTransform(const btTransform& worldTrans) override final
        {
            m_transformChanged = true;
            worldTrans.getOpenGLMatrix(reinterpret_cast<float*>(&m_transform));
        }

    private:
        glm::mat4& m_transform;
        bool m_transformChanged = false;
    };
}
