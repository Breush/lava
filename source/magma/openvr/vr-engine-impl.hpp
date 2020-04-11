#pragma once

#include <lava/magma/vr-engine.hpp>

namespace lava::magma {
    class VrEngine::Impl {
    public:
        Impl(VrEngine& engine);

        bool init();
        void update();
        std::optional<VrEvent> pollEvent();
        VrRenderingNeeds renderingNeeds(VrRenderingNeedsInfo info) const;

        // Devices
        Mesh& deviceMesh(VrDeviceType deviceType, Scene& scene) const;

        // View
        Extent2d renderTargetExtent() const;
        glm::mat4 eyeProjectionTransform(VrEye eye, float nearClip, float farClip) const;
        glm::mat4 eyeToHeadTransform(VrEye eye) const;

    private:
        VrEngine& m_engine;

        vr::IVRSystem* m_vrSystem = nullptr;
        std::vector<vr::TrackedDevicePose_t> m_devicesPoses;

        const glm::mat4 m_fixesTransform = glm::mat4(-1, 0, 0, 0, // -X
                                                     0, 0, 1, 0,  // Z
                                                     0, 1, 0, 0,  // Y
                                                     0, 0, 0, 1);

        // @note VrEngine::DeviceInfo.data
        // [0] uint32_t index = 0;                        // OpenVR's device index.
        // [1] vr::ETrackedDeviceClass type;              // Which device is being tracked.
    };
}
