#pragma once

#include <lava/core/transform.hpp>
#include <lava/core/vr-device-type.hpp>

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    /**
     * Basically wraps all functionalities from magma::VrEngine's API.
     */
    class VrManager {
    public:
        VrManager(GameEngine& engine);

        /// Called by game engine to update meshes.
        void update();

        /// Whether a VR system can be used (initialization worked).
        bool enabled() const;

        /// Get whether a device is valid (active and ready to be asked for transform or mesh).
        bool deviceValid(VrDeviceType deviceType) const;

        /// Get a device transform.
        const lava::Transform& deviceTransform(VrDeviceType deviceType) const;

        /// The world translation of the VR area.
        const glm::vec3& translation() const { return m_tranlation; }
        void translation(const glm::vec3& translation);

    private:
        GameEngine& m_engine;

        glm::vec3 m_tranlation{0.f};
    };
}
