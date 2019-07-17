#include "./vr-render-target-impl.hpp"

#include <lava/magma/camera.hpp>
#include <lava/magma/render-engine.hpp>

#include "../../aft-vulkan/camera-aft.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

VrRenderTarget::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
    , m_fence(m_engine.device())
{
    if (!m_engine.vrEnabled()) {
        logger.error("magma.vulkan.vr-render-target") << "Cannot use VrRenderTarget because "
                                                      << "no VR system has been enabled on startup." << std::endl;
    }
}

VrRenderTarget::Impl::~Impl()
{
    cleanup();
}

//----- IRenderTarget

void VrRenderTarget::Impl::init(uint32_t id)
{
    m_id = id;

    initFence();

    // @note This is only the minimum recommended size, but this can be configurable.
    m_extent = m_engine.vrEngine().renderTargetExtent();

    logger.info("magma.vulkan.vr-render-target")
        << "VR system recommend a size of " << m_extent.width << "x" << m_extent.height << std::endl;
}

bool VrRenderTarget::Impl::prepare()
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    // Waiting for previous rendering to be over
    {
        PROFILE_BLOCK("VrRenderTarget - waitForFences", PROFILER_COLOR_DRAW);

        static const auto MAX = std::numeric_limits<uint64_t>::max();
        m_engine.device().waitForFences(1u, &m_fence, true, MAX);
        m_engine.device().resetFences(1u, &m_fence);
    }

    m_leftEyeCameraController.updateCamera(VrEye::Left);
    m_rightEyeCameraController.updateCamera(VrEye::Right);

    return true;
}

void VrRenderTarget::Impl::render(vk::CommandBuffer commandBuffer)
{
    // Change final image layout
    m_leftEyeCamera->aft().changeImageLayout(vk::ImageLayout::eTransferSrcOptimal, commandBuffer);
    m_rightEyeCamera->aft().changeImageLayout(vk::ImageLayout::eTransferSrcOptimal, commandBuffer);
}

void VrRenderTarget::Impl::draw(const std::vector<vk::CommandBuffer>& commandBuffers) const
{
    PROFILE_FUNCTION(PROFILER_COLOR_DRAW);

    vr::VRTextureBounds_t bounds;
    bounds.uMin = 0.0f;
    bounds.uMax = 1.0f;
    bounds.vMin = 0.0f;
    bounds.vMax = 1.0f;

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    if (m_engine.graphicsQueue().submit(1, &submitInfo, m_fence) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.vr-render-target") << "Failed to submit draw command buffer." << std::endl;
    }

    // Submit to SteamVR
    vr::VRVulkanTextureData_t vulkanData;
    vulkanData.m_nImage = (uint64_t) static_cast<VkImage>(m_leftEyeCamera->renderImage().impl().image());
    vulkanData.m_pDevice = m_engine.device();
    vulkanData.m_pPhysicalDevice = m_engine.physicalDevice();
    vulkanData.m_pInstance = m_engine.instance();
    vulkanData.m_pQueue = m_engine.graphicsQueue();
    vulkanData.m_nQueueFamilyIndex = m_engine.graphicsQueueFamilyIndex();

    vulkanData.m_nWidth = m_extent.width;
    vulkanData.m_nHeight = m_extent.height;
    vulkanData.m_nFormat = VK_FORMAT_R8G8B8A8_SRGB; // @note Don't know really what's right for these 2 parameters.
    vulkanData.m_nSampleCount = 1;

    vr::Texture_t texture = {&vulkanData, vr::TextureType_Vulkan, vr::ColorSpace_Auto};
    vr::EVRCompositorError error = vr::VRCompositor()->Submit(vr::Eye_Left, &texture, &bounds);

    if (error != 0) {
        logger.warning("magma.vulkan.vr-render-target") << "Rendering with error: " << error << std::endl;
    }

    vulkanData.m_nImage = (uint64_t) static_cast<VkImage>(m_rightEyeCamera->renderImage().impl().image());
    error = vr::VRCompositor()->Submit(vr::Eye_Right, &texture, &bounds);

    if (error != 0) {
        logger.warning("magma.vulkan.vr-render-target") << "Rendering with error: " << error << std::endl;
    }
}

//----- VrRenderTarget

void VrRenderTarget::Impl::bindScene(RenderScene& scene)
{
    m_scene = &scene.impl();

    cleanup();

    // @todo Work on a stereo render pass for VR
    m_leftEyeCamera = &scene.make<Camera>(m_extent);
    m_leftEyeCameraController.bind(*m_leftEyeCamera);
    m_rightEyeCamera = &scene.make<Camera>(m_extent);
    m_rightEyeCameraController.bind(*m_rightEyeCamera);

    // Let the right eye use the same shadow maps than the left eye
    m_scene->shadowsFallbackCamera(*m_rightEyeCamera, *m_leftEyeCamera);
}

//----- Internal

void VrRenderTarget::Impl::cleanup()
{
    if (m_leftEyeCamera != nullptr) {
        m_scene->remove(*m_leftEyeCamera);
    }
    if (m_rightEyeCamera != nullptr) {
        m_scene->remove(*m_rightEyeCamera);
    }
}

void VrRenderTarget::Impl::initFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    auto result = m_engine.device().createFence(&fenceInfo, nullptr, m_fence.replace());
    if (result != vk::Result::eSuccess) {
        logger.error("magma.vulkan.window-render-target") << "Unable to create fence." << std::endl;
    }
}
