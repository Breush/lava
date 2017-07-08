#pragma once

#include <lava/magma/render-engine.hpp>
#include <vulkan/vulkan.hpp>

#include "../capsule.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the G-Buffer construction.
     */
    class GBuffer final {
    public:
        GBuffer(RenderEngine::Impl& engine);

        void init();

    protected:
        void createGraphicsPipeline();

    private:
        RenderEngine::Impl& m_engine;

        // Graphics pipeline
        vulkan::CapsulePP<vk::PipelineLayout> m_pipelineLayout;
        vulkan::CapsulePP<vk::RenderPass> m_renderPass;
        vulkan::CapsulePP<vk::Pipeline> m_graphicsPipeline;
    };
}
