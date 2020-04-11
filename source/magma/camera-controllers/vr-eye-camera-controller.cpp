#include <lava/magma/camera-controllers/vr-eye-camera-controller.hpp>

#include <lava/magma/camera.hpp>
#include <lava/magma/scene.hpp>
#include <lava/magma/render-engine.hpp>

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

    auto& vrEngine = m_camera->scene().engine().vr();

    auto projectionTransform = vrEngine.eyeProjectionTransform(eye, m_camera->nearClip(), m_camera->farClip());
    m_camera->projectionTransform(projectionTransform);

    auto viewTransform = vrEngine.eyeViewTransform(eye);
    m_camera->viewTransform(viewTransform);
}
