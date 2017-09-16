#pragma once

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    class PipelineHolder;
}

namespace lava::magma::vulkan {
    /**
     * Holds a vulkan::RenderPass.
     */
    class RenderPassHolder final {
    public:
        RenderPassHolder(RenderEngine::Impl& engine);

        void init();

        /// Add a new subpass. Order is important.
        void add(PipelineHolder& pipelineHolder);

        vk::RenderPass renderPass() const { return m_renderPass; }

    private:
        // References
        RenderEngine::Impl& m_engine;

        // Render pass
        vulkan::RenderPass m_renderPass;

        // Configuration
        std::vector<PipelineHolder*> m_pipelineHolders;
    };
}
