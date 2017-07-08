#pragma once

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    struct UserDataRenderIn {
        const vk::CommandBuffer* commandBuffer;
        const vk::PipelineLayout* pipelineLayout;
    };
}
