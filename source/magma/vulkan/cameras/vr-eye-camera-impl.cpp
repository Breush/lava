#include "./vr-eye-camera-impl.hpp"

#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

VrEyeCamera::Impl::Impl(RenderScene& scene, Extent2d extent)
    : m_scene(scene.impl())
{
    this->extent(extent);
}

VrEyeCamera::Impl::~Impl() {}

//----- ICamera

RenderImage VrEyeCamera::Impl::renderImage() const
{
    return m_scene.cameraRenderImage(m_id);
}

RenderImage VrEyeCamera::Impl::depthRenderImage() const
{
    return m_scene.cameraDepthRenderImage(m_id);
}

void VrEyeCamera::Impl::polygonMode(PolygonMode polygonMode)
{
    m_polygonMode = polygonMode;

    if (m_initialized) {
        m_scene.updateCamera(m_id);
    }
}

//----- ICamera::Impl

void VrEyeCamera::Impl::init(uint32_t id)
{
    m_id = id;

    m_initialized = true;
    updateBindings();
}

void VrEyeCamera::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                               uint32_t pushConstantOffset) const
{
    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                pushConstantOffset, sizeof(vulkan::CameraUbo), &m_ubo);
}

void VrEyeCamera::Impl::extent(Extent2d extent)
{
    // Ignoring resize with same extent
    if (m_extent.width == extent.width && m_extent.height == extent.height) {
        return;
    }

    m_extent.width = extent.width;
    m_extent.height = extent.height;

    if (m_initialized) {
        m_scene.updateCamera(m_id);
    }
}

//----- Internal interface

void VrEyeCamera::Impl::update(VrEngine::Eye eye)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto& vrEngine = m_scene.engine().vrEngine();

    auto projectionTransform = vrEngine.eyeProjectionTransform(eye);
    forceProjectionTransform(projectionTransform);

    auto viewTransform = vrEngine.eyeViewTransform(eye);
    forceViewTransform(viewTransform);
}

void VrEyeCamera::Impl::forceProjectionTransform(const glm::mat4& projectionTransform)
{
    m_projectionTransform = projectionTransform;
}

void VrEyeCamera::Impl::forceViewTransform(glm::mat4 viewTransform)
{
    m_translation = glm::vec3(viewTransform[3]);
    m_viewTransform = glm::inverse(viewTransform);

    updateFrustum();
    updateBindings();
}

void VrEyeCamera::Impl::changeImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    m_scene.changeCameraRenderImageLayout(m_id, imageLayout, commandBuffer);
}

//----- Private

void VrEyeCamera::Impl::updateFrustum()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    /* @fixme Does not work - frustum culling disabled for this camera */
}

void VrEyeCamera::Impl::updateBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto transposeViewTransform = glm::transpose(m_viewTransform * m_scene.engine().vrEngine().fixesTransform());
    m_ubo.viewTransform0 = transposeViewTransform[0];
    m_ubo.viewTransform1 = transposeViewTransform[1];
    m_ubo.viewTransform2 = transposeViewTransform[2];

    m_ubo.projectionFactors0[0] = m_projectionTransform[0][0];
    m_ubo.projectionFactors0[1] = m_projectionTransform[1][1];
    m_ubo.projectionFactors0[2] = m_projectionTransform[2][2];
    m_ubo.projectionFactors0[3] = m_projectionTransform[3][2];
    m_ubo.projectionFactors1[0] = m_projectionTransform[2][0];
    m_ubo.projectionFactors1[1] = m_projectionTransform[2][1];
    m_ubo.projectionFactors1[2] = m_extent.width;
    m_ubo.projectionFactors1[3] = m_extent.height;
}
