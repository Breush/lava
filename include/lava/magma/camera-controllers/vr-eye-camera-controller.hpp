#pragma once

#include <glm/glm.hpp>
#include <lava/magma/vr-eye.hpp>

namespace lava::magma {
    class Camera;
}

namespace lava::magma {
    /**
     * Camera controller for VR rendering, mostly used internally.
     */
    class VrEyeCameraController {
    public:
        VrEyeCameraController() {}
        VrEyeCameraController(Camera& camera) { bind(camera); }

        /// Bind a camera to this controller.
        void bind(Camera& camera);

        /// Update the camera according to the specified eye of VR headset.
        void updateCamera(VrEye eye);

    private:
        Camera* m_camera;
    };
}
