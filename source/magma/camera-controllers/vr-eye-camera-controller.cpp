#include <lava/magma/camera-controllers/vr-eye-camera-controller.hpp>

#include <lava/magma/camera.hpp>

#include "../vulkan/render-engine-impl.hpp" // @fixme THIS IS BACKEND DEPENDENT, BUT SHOULD NOT!

using namespace lava::chamber;
using namespace lava::magma;

void VrEyeCameraController::bind(Camera& camera)
{
    m_camera = &camera;

    camera.vrAimed(true);
}

void VrEyeCameraController::updateCamera(VrEye eye)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // @fixme THIS IS GOING THOUGH ENGINE IMPL, and should not,
    // as VR Engine is backend independant, we should just put it in
    // "fore" engine.
    auto& vrEngine = m_camera->scene().engine().impl().vrEngine();

    auto projectionTransform = vrEngine.eyeProjectionTransform(eye, m_camera->nearClip(), m_camera->farClip());
    m_camera->projectionTransform(projectionTransform);

    auto viewTransform = vrEngine.eyeViewTransform(eye);
    m_camera->viewTransform(glm::inverse(viewTransform) * m_camera->scene().engine().impl().vrEngine().fixesTransform());
}
