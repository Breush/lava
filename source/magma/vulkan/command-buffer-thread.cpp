#include "./command-buffer-thread.hpp"

#include "../vulkan/render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

CommandBufferThread::CommandBufferThread(RenderEngine::Impl& engine, const char* threadName)
    : m_engine(engine)
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

        vk::CommandPoolCreateInfo createInfo;
        createInfo.queueFamilyIndex = queueFamilyIndices.graphics;
        createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        auto result = m_engine.device().createCommandPoolUnique(createInfo);
        m_commandPool = vulkan::checkMove(result, "command-buffer-thread", "Unable to create command pool.");
    });
    wait();
}

void CommandBufferThread::createCommandBuffers()
{
    job([&] {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.commandPool = m_commandPool.get();
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = m_commandBuffers.size();

        auto result = m_engine.device().allocateCommandBuffersUnique(allocInfo);
        auto commandBuffers = vulkan::checkMove(result, "command-buffer-thread", "Unable to create command buffers.");
        for (auto i = 0u; i < commandBuffers.size(); ++i) {
            m_commandBuffers[i] = std::move(commandBuffers[i]);
        }
    });
    wait();
}
