#include "./camera-aft.hpp"

#include <lava/magma/camera.hpp>
#include <lava/magma/scene.hpp>

#include "./scene-aft.hpp"

using namespace lava::magma;

CameraAft::CameraAft(Camera& fore, Scene& scene)
    : m_fore(fore)
    , m_scene(scene)
{
}

void CameraAft::render(vk::CommandBuffer commandBuffer, PipelineKind pipelineKind, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset) const
{
    if (pipelineKind == PipelineKind::Graphics) {
        commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                    pushConstantOffset, sizeof(CameraUbo), &m_fore.ubo());
    } else { // PipelineKind::RayTracing
        commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eRaygenKHR,
                                    pushConstantOffset, sizeof(CameraUbo), &m_fore.ubo());
    }
}

void CameraAft::changeImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    m_scene.aft().changeCameraRenderImageLayout(m_fore, imageLayout, commandBuffer);
}

// ----- Fore

RenderImage CameraAft::foreRenderImage() const
{
    return m_scene.aft().cameraRenderImage(m_fore);
}

RenderImage CameraAft::foreDepthRenderImage() const
{
    return m_scene.aft().cameraDepthRenderImage(m_fore);
}

void CameraAft::foreExtentChanged()
{
    m_scene.aft().updateCamera(m_fore);
}

void CameraAft::forePolygonModeChanged()
{
    m_scene.aft().updateCamera(m_fore);
}
