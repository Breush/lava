#pragma once

#include "./render-engine-impl.hpp"
#include "./stages/i-renderer-stage.hpp"

namespace lava::magma::vulkan {
    /**
     * A thread that update a command buffer on-demand.
     * It encapsulates it own command pool and command buffers.
     */
    class CommandBufferThread final : public chamber::Thread {
    public:
        CommandBufferThread(RenderEngine::Impl& engine, const char* threadName = "");

        template <class Stage>
        void record(Stage& stage)
        {
            job([&] {
                vk::CommandBufferBeginInfo beginInfo;
                beginInfo.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue;

                m_commandBuffer.begin(&beginInfo);
                // @fixme We should rename render -> record
                stage.render(m_commandBuffer);
                m_commandBuffer.end();
            });
        }

        vk::CommandBuffer commandBuffer() const { return m_commandBuffer; }

    protected:
        void createCommandPool();
        void createCommandBuffers();

    private:
        // References
        RenderEngine::Impl& m_engine;

        // Resources
        vulkan::CommandPool m_commandPool;
        vk::CommandBuffer m_commandBuffer;
    };
}
