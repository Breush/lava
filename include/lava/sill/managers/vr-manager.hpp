#pragma once

#include <glm/mat4x4.hpp>
#include <lava/core/vr-device-type.hpp>

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    /**
     * Basically wraps all functionalities from magma::RenderEngine's VR API.
     */
    class VrManager {
    public:
        VrManager(GameEngine& engine);

        /// The world translation of the VR area.
        const glm::vec3& translation() const;
        void translation(const glm::vec3& translation);

        /// Whether a VR system can be used (initialization worked).
        bool enabled() const;

        /// Get whether a device is valid (active and ready to be asked for transform or mesh).
        bool deviceValid(VrDeviceType deviceType) const;

        // @todo Could be nice to access a VrDevice entity instead of transform directly.
        /// Get a device transform.
        const glm::mat4& deviceTransform(VrDeviceType deviceType) const;

    private:
        GameEngine& m_engine;
    };
}
