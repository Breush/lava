#pragma once

#include <lava/chamber/thread.hpp>
#include <lava/magma/render-engine.hpp>

#include "./stages/i-renderer-stage.hpp"
#include "./wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * A thread that update a command buffer on-demand.
     * It encapsulates it own command pool and command buffers.
     *
     * It holds multiple command buffers so that one can
     * record twice without issue.
     */
    class CommandBufferThread final : public chamber::Thread {
    public:
        CommandBufferThread(RenderEngine::Impl& engine, const char* threadName = "");

        // @note We can't take a IRendererStage because of ShadowsStage not being one.
        template <class Stage, class... Args>
        void record(Stage& stage, Args&... args)
        {
            m_bufferIndex = (m_bufferIndex + 1) % m_commandBuffers.size();
            auto& commandBuffer = m_commandBuffers[m_bufferIndex];

            // @note We pass args by copy, ensuring that it does not rely on dead reference
            job([=, &stage, &commandBuffer] {
                vk::CommandBufferInheritanceInfo inheritanceInfo;
                inheritanceInfo.renderPass = stage.renderPass();
                inheritanceInfo.subpass = 0u;

                // @note SimultaneousUse is needed because the same
                // camera can be used to render into different windows.
                vk::CommandBufferBeginInfo beginInfo;
                beginInfo.flags =
                    vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eSimultaneousUse;
                beginInfo.pInheritanceInfo = &inheritanceInfo;

                commandBuffer.begin(&beginInfo);
                stage.record(commandBuffer, args...);
                commandBuffer.end();
            });
        }

        vk::CommandBuffer commandBuffer() const { return m_commandBuffers[m_bufferIndex]; }

    protected:
        void createCommandPool();
        void createCommandBuffers();

    private:
        // References
        RenderEngine::Impl& m_engine;

        // Resources
        vulkan::CommandPool m_commandPool;
        std::array<vk::CommandBuffer, 2> m_commandBuffers;
        uint32_t m_bufferIndex = 0u; //!< Holds which command buffer has been used for last record.
    };
}
