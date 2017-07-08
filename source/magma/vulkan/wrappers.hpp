#pragma once

#include <vulkan/vulkan.hpp>

#include "./capsule.hpp"

namespace lava::magma::vulkan {
    $capsule_device(RenderPass);
    $capsule_device(PipelineLayout);
    $capsule_device(Pipeline);
}
