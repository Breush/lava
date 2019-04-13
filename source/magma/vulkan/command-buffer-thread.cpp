#include "./command-buffer-thread.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

CommandBufferThread::CommandBufferThread(RenderEngine::Impl& engine, const char* threadName)
    : m_engine(engine)
    , m_commandPool(m_engine.device())
{
    job([&] { chamber::profilerThreadName(threadName); });

    createCommandPool();
    createCommandBuffers();
}

void CommandBufferThread::createCommandPool()
{
    job([&] {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        auto queueFamilyIndices = vulkan::findQueueFamilies(m_engine.physicalDevice(), nullptr);

        vk::CommandPoolCreateInfo poolInfo;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        if (m_engine.device().createCommandPool(&poolInfo, nullptr, m_commandPool.replace()) != vk::Result::eSuccess) {
            logger.error("magma.vulkan.command-buffer-thread") << "Failed to create command pool." << std::endl;
        }
    });
    wait();
}

void CommandBufferThread::createCommandBuffers()
{
    job([&] {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.commandPool = m_commandPool;
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = m_commandBuffers.size();

        if (m_engine.device().allocateCommandBuffers(&allocateInfo, m_commandBuffers.data()) != vk::Result::eSuccess) {
            logger.error("magma.vulkan.command-buffer-thread") << "Failed to create command buffers." << std::endl;
        }
    });
    wait();
}
