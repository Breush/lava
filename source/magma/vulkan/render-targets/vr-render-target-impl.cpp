#include "./vr-render-target-impl.hpp"

#include <lava/magma/camera.hpp>
#include <lava/magma/render-engine.hpp>

#include "../../aft-vulkan/camera-aft.hpp"
#include "../../aft-vulkan/scene-aft.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

VrRenderTarget::Impl::Impl(RenderEngine& engine)
    : m_engine(engine)
{
    if (!m_engine.vr().enabled()) {
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
    m_extent = m_engine.vr().renderTargetExtent();

    logger.info("magma.vulkan.vr-render-target")
        << "VR system recommend a size of " << m_extent.width << "x" << m_extent.height << std::endl;
}

void VrRenderTarget::Impl::update()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_leftEyeCameraController.updateCamera(VrEye::Left);
    m_rightEyeCameraController.updateCamera(VrEye::Right);
}

bool VrRenderTarget::Impl::prepare()
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    // Waiting for previous rendering to be over
    {
        PROFILE_BLOCK("VrRenderTarget - waitForFences", PROFILER_COLOR_DRAW);

        static const auto MAX = std::numeric_limits<uint64_t>::max();
        m_engine.impl().device().waitForFences(1u, &m_fence.get(), true, MAX);
        m_engine.impl().device().resetFences(1u, &m_fence.get());
    }

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

    if (m_engine.impl().graphicsQueue().submit(1, &submitInfo, m_fence.get()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.vr-render-target") << "Failed to submit draw command buffer." << std::endl;
    }

    // Submit to SteamVR
    vr::VRVulkanTextureData_t vulkanData;
    vulkanData.m_pDevice = m_engine.impl().device();
    vulkanData.m_pPhysicalDevice = m_engine.impl().physicalDevice();
    vulkanData.m_pInstance = m_engine.impl().instance();
    vulkanData.m_pQueue = m_engine.impl().graphicsQueue();
    vulkanData.m_nQueueFamilyIndex = m_engine.impl().graphicsQueueFamilyIndex();

    vulkanData.m_nWidth = m_extent.width;
    vulkanData.m_nHeight = m_extent.height;
    vulkanData.m_nSampleCount = 1u;

    vulkanData.m_nImage = (uint64_t) static_cast<VkImage>(m_leftEyeCamera->renderImage().impl().image());
    vulkanData.m_nFormat = (uint32_t) static_cast<VkImageLayout>(m_leftEyeCamera->renderImage().impl().layout());

    vr::Texture_t texture = {&vulkanData, vr::TextureType_Vulkan, vr::ColorSpace_Auto};
    vr::EVRCompositorError error = vr::VRCompositor()->Submit(vr::Eye_Left, &texture, &bounds);

    if (error != 0) {
        logger.warning("magma.vulkan.vr-render-target") << "Rendering with error: " << error << std::endl;
    }

    vulkanData.m_nImage = (uint64_t) static_cast<VkImage>(m_rightEyeCamera->renderImage().impl().image());
    vulkanData.m_nFormat = (uint32_t) static_cast<VkImageLayout>(m_rightEyeCamera->renderImage().impl().layout());

    vr::VRCompositor()->WaitGetPoses(nullptr, 0, nullptr, 0);
    error = vr::VRCompositor()->Submit(vr::Eye_Right, &texture, &bounds);

    if (error != 0) {
        logger.warning("magma.vulkan.vr-render-target") << "Rendering with error: " << error << std::endl;
    }
}

//----- VrRenderTarget

void VrRenderTarget::Impl::bindScene(Scene& scene)
{
    m_scene = &scene;

    cleanup();

    // @todo Work on a stereo render pass for VR
    m_leftEyeCamera = &scene.make<Camera>(m_extent);
    m_leftEyeCameraController.bind(*m_leftEyeCamera);
    m_rightEyeCamera = &scene.make<Camera>(m_extent);
    m_rightEyeCameraController.bind(*m_rightEyeCamera);

    // Let the right eye use the same shadow maps than the left eye
    m_scene->aft().shadowsFallbackCamera(*m_rightEyeCamera, *m_leftEyeCamera);
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
    vk::FenceCreateInfo createInfo;
    createInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    auto result = m_scene->engine().impl().device().createFenceUnique(createInfo);
    m_fence = vulkan::checkMove(result, "vr-render-target", "Unable to create fence.");
}
