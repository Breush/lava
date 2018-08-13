#pragma once

#include <bullet/btBulletDynamicsCommon.h>
#include <glm/mat4x4.hpp>

namespace lava::dike {
    class MotionState final : public btMotionState {
    public:
        // This referenced transform will be automatically updated
        MotionState(glm::mat4& transform)
            : m_transform(transform)
        {
        }

        virtual ~MotionState() = default;

        void getWorldTransform(btTransform& worldTrans) const override final
        {
            // @note We don't really care here... That's the initial position
            // of the motion state, but we set it via the rigidBodies directly.
            worldTrans.setIdentity();
        }

        void setWorldTransform(const btTransform& worldTrans) override final
        {
            // @note Bullet sometimes provides an identity transform,
            // on the first update. If this happens, we just ignore it,
            // waiting for the next update. Otherwise, there might be
            // visual glitches.
            if (m_firstTime) {
                m_firstTime = false;
                float transform[16];
                worldTrans.getOpenGLMatrix(transform);
                if (transform[0] == 1.f && transform[1] == 0.f && transform[2] == 0.f && transform[3] == 0.f
                    && transform[4] == 0.f && transform[5] == 1.f && transform[6] == 0.f && transform[7] == 0.f
                    && transform[8] == 0.f && transform[9] == 0.f && transform[10] == 1.f && transform[11] == 0.f
                    && transform[12] == 0.f && transform[13] == 0.f && transform[14] == 0.f && transform[15] == 1.f) {
                    return;
                }
            }

            worldTrans.getOpenGLMatrix(reinterpret_cast<float*>(&m_transform));
        }

    private:
        glm::mat4& m_transform;
        bool m_firstTime = true;
    };
}
