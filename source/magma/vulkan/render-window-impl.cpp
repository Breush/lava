#include "./render-window-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <lava/magma/render-engine.hpp>

#include "./render-engine-impl.hpp"
#include "./swapchain.hpp"

using namespace lava;

RenderWindow::Impl::Impl(VideoMode mode, const std::string& title)
    : m_window(mode, title)
    , m_windowExtent({mode.width, mode.height})
{
}

//----- IRenderTarget

void RenderWindow::Impl::init(RenderEngine& engine)
{
    m_engine = &engine.impl();

    m_swapchain = &m_engine->swapchain(); // @todo Create one here instead

    // @todo Add required extensions etc.

    m_engine->m_windowHandle = windowHandle();
    m_engine->m_windowExtent = m_windowExtent;

    m_engine->initVulkan(); // @todo Wait what?
}

void RenderWindow::Impl::draw() const
{
    if (!m_engine) {
        logger.warning("magma.render-window") << "Attempt to draw without an engine binded." << std::endl;
        return;
    }

    /*uint32_t imageIndex;
    const auto MAX = std::numeric_limits<uint64_t>::max();
    auto result =
        vkAcquireNextImageKHR(m_engine->device(), *m_swapchain, MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_engine->recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        logger.error("magma.vulkan.draw") << "Failed to acquire swapchain image." << std::endl;
    }

    // Submit to the queue
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        logger.error("magma.vulkan.layer") << "Failed to submit draw command buffer." << std::endl;
        exit(1);
    }

    // Submitting the image back to the swap chain
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);*/
}

void RenderWindow::Impl::refresh()
{
    m_engine->recreateSwapchain();
}

bool RenderWindow::Impl::pollEvent(Event& event)
{
    return m_window.pollEvent(event);
}

void RenderWindow::Impl::close()
{
    m_window.close();
}

WindowHandle RenderWindow::Impl::windowHandle() const
{
    return m_window.windowHandle();
}

VideoMode RenderWindow::Impl::videoMode() const
{
    return m_window.videoMode();
}

void RenderWindow::Impl::videoMode(const VideoMode& mode)
{
    m_windowExtent = {mode.width, mode.height};
    m_engine->recreateSwapchain();
}

bool RenderWindow::Impl::opened() const
{
    return m_window.opened();
}
