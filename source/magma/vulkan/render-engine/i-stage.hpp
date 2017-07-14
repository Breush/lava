#pragma once

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

namespace lava::magma {
    class IStage {
    public:
        IStage(RenderEngine::Impl& engine);

        /// Called once in a runtime.
        virtual void init() = 0;

        /// Called each time the extent changes.
        virtual void update() = 0;

        /// Called each frame.
        virtual void render(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex) = 0;

    protected:
        RenderEngine::Impl& m_engine;

        // Render pipeline
        vulkan::RenderPass m_renderPass;
        vulkan::PipelineLayout m_pipelineLayout;
        vulkan::Pipeline m_pipeline;
    };
}
