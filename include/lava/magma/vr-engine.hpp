#pragma once

#include <lava/core/extent.hpp>
#include <lava/core/macros/aft.hpp>
#include <lava/core/vr-device-type.hpp>
#include <lava/core/vr-event.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/vr-eye.hpp>
#include <lava/magma/vr-rendering-needs.hpp>

namespace lava::magma {
    class VrEngine {
    public:
        struct DeviceInfo {
            bool valid = false;             // Whether the device is valid.
            lava::Transform transform;      // Last known world-space transform of the device.
            // Controller specific data
            float vibrationEnabled = false; // Whether we should vibrate this frame.
            float vibrationPower = 0.8f;    // How much to vibrate. From 0 to 1.
            // Back-end specific data
            uint32_t data[2u];
        };

    public:
        VrEngine();
        ~VrEngine();

        /// Try to start the VR system. Will set enabled() to true on success.
        void init();

        /// Update transform of all devices.
        void update();

        /// Whether a VR system can be used (initialization worked).
        bool enabled() const { return m_enabled; }

        /// Poll VR event.
        std::optional<VrEvent> pollEvent();

        /// Requirements to interface with rendering back-end.
        VrRenderingNeeds renderingNeeds(VrRenderingNeedsInfo info) const;

        /**
         * @name Devices
         */
        /// @{
        /// Get whether a device is valid (active and ready to be asked for transform or mesh).
        bool deviceValid(VrDeviceType deviceType) const { return m_devicesInfos.at(deviceType).valid; }

        /// Get a device transform.
        const lava::Transform& deviceTransform(VrDeviceType deviceType) const { return m_devicesInfos.at(deviceType).transform; }

        /**
         * Get a device mesh.
         * Can be requested only if deviceValid() is true.
         *
         * @note Requesting VrDeviceType::Head will set
         * vrRenderable() to false for this mesh, consider tweaking
         * this parameter if you need to render it within a VrRenderTarget.
         *
         * @note The first time this function is called, it will make
         * a mesh within the provided scene. This means the scene
         * cannot be removed afterwards or the mesh used in a different scene.
         */
        Mesh& deviceMesh(VrDeviceType deviceType, Scene& scene);

        /// The general infos of the device.
        const DeviceInfo& deviceInfo(VrDeviceType deviceType) const { return m_devicesInfos.at(deviceType); }
        DeviceInfo& deviceInfo(VrDeviceType deviceType) { return m_devicesInfos[deviceType]; }
        /// @}

        /**
         * @name Area
         */
        /// @{
        /// World matrix of the VR area.
        // @todo Shouldn't we pass a transform instead?
        const glm::mat4& matrix() const { return m_matrix; }
        void matrix(const glm::mat4& matrix) { m_matrix = matrix; }
        /// @}

        /**
         * @name View
         */
        /// @{
        /// Recommended render target size.
        Extent2d renderTargetExtent() const;

        /// Get camera projection matrix for an eye.
        glm::mat4 eyeProjectionMatrix(VrEye eye, float nearClip, float farClip) const;

        /// Get premultiplied (headTransform * eyeToHeadTransform) eye world-space transform.
        lava::Transform eyeViewTransform(VrEye eye) const;
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }
        const Impl& impl() const { return *m_impl; }

    private:
        Impl* m_impl = nullptr;

        // Resources
        bool m_enabled = false;
        std::unordered_map<VrDeviceType, DeviceInfo> m_devicesInfos;
        std::unordered_map<VrDeviceType, Mesh*> m_devicesMeshes;

        glm::mat4 m_matrix = glm::mat4(1.f);
    };
}
