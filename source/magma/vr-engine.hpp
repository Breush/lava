#pragma once

#include <lava/core/extent.hpp>
#include <lava/core/vr-device-type.hpp>
#include <lava/core/vr-event.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/render-engine.hpp>
#include <lava/magma/vr-eye.hpp>

namespace lava::magma {
    /**
     * This VrEngine is backend independent.
     */
    class VrEngine {
    public:
        VrEngine();
        ~VrEngine();

        // ----- RenderEngine API

        bool enabled() const { return m_enabled; }
        bool deviceValid(VrDeviceType deviceType) const { return m_devicesInfos.at(deviceType).valid; }
        const glm::mat4& deviceTransform(VrDeviceType deviceType) const { return m_devicesInfos.at(deviceType).fixedTransform; }
        Mesh& deviceMesh(VrDeviceType deviceType, Scene& scene);
        const glm::vec3& translation() const { return m_translation; }
        void translation(const glm::vec3& translation);
        std::optional<VrEvent> pollEvent();

        // ----- Internal API

        /// Try to start the VR system. Will set enabled() to true on success.
        void init();

        /// Update transform of all devices.
        void update();

        /// Recommended render target size.
        Extent2d renderTargetExtent();

        /// Get camera projection transform for an eye.
        glm::mat4 eyeProjectionTransform(VrEye eye, float nearClip, float farClip);

        /// Get camera view transform for an eye from the head.
        glm::mat4 eyeToHeadTransform(VrEye eye);

        /// Get premultiplied (headTransform * eyeToHeadTransform) eye absolute transform.
        glm::mat4 eyeViewTransform(VrEye eye);

        /// Need to adapt from OpenVR.
        const glm::mat4 fixesTransform() const { return m_fixesTransform; }

    protected:
        struct DeviceInfo {
            uint32_t index = 0;                        // OpenVR's device index.
            bool valid = false;                        // Whether the device is valid.
            vr::ETrackedDeviceClass type;              // Which device is being tracked.
            glm::mat4 transform = glm::mat4(1.f);      // Last known VR-space transform of the device given by OpenVR.
            glm::mat4 fixedTransform = glm::mat4(1.f); // Last known absolute transform of the device in our coordinate system.
        };

    private:
        // Resources
        bool m_enabled = false;
        vr::IVRSystem* m_vrSystem = nullptr;
        std::vector<vr::TrackedDevicePose_t> m_devicesPoses;
        std::unordered_map<VrDeviceType, DeviceInfo> m_devicesInfos;
        std::unordered_map<VrDeviceType, Mesh*> m_devicesMeshes;

        glm::mat4 m_transform = glm::mat4(1.f);
        glm::mat4 m_unfixedTransform = glm::mat4(1.f); // Allows transform within openVR space.
        glm::vec3 m_translation = glm::vec3(0.f);

        const glm::mat4 m_fixesTransform = glm::mat4(-1, 0, 0, 0, // -X
                                                     0, 0, 1, 0,  // Z
                                                     0, 1, 0, 0,  // Y
                                                     0, 0, 0, 1);
    };
}
