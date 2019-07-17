#include "./camera-aft.hpp"

#include <lava/magma/camera.hpp>

#include "../vulkan/render-scenes/render-scene-impl.hpp"

using namespace lava::magma;

CameraAft::CameraAft(Camera& fore, RenderScene::Impl& scene)
    : m_fore(fore)
    , m_scene(scene)
{
}

void CameraAft::init(uint32_t id)
{
    m_id = id;
}

void CameraAft::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset) const
{
    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                pushConstantOffset, sizeof(CameraUbo), &m_fore.ubo());
}

void CameraAft::changeImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    m_scene.changeCameraRenderImageLayout(m_id, imageLayout, commandBuffer);
}

// ----- Fore

RenderImage CameraAft::foreRenderImage() const
{
    return m_scene.cameraRenderImage(m_id);
}

RenderImage CameraAft::foreDepthRenderImage() const
{
    return m_scene.cameraDepthRenderImage(m_id);
}

void CameraAft::foreExtentChanged()
{
    m_scene.updateCamera(m_id);
}

void CameraAft::forePolygonModeChanged()
{
    m_scene.updateCamera(m_id);
}
