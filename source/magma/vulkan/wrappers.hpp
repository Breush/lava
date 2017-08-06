#pragma once

#include <vulkan/vulkan.hpp>

#include "./capsule.hpp"

namespace lava::magma::vulkan {
    $capsule_instance(SurfaceKHR);

    $capsule_device(SwapchainKHR);

    $capsule_device(RenderPass);
    $capsule_device(PipelineLayout);
    $capsule_device(Pipeline);

    $capsule_device(Image);
    $capsule_device(Buffer);
    $capsule_device(ImageView);
    $capsule_device(BufferView);
    $capsule_device(Framebuffer);
    $capsule_device(DeviceMemory, freeMemory);

    $capsule_device(DescriptorSetLayout);
    $capsule_device(DescriptorPool);

    $capsule_device(Semaphore);
    $capsule_device(CommandPool);

    $capsule_device(ShaderModule);
}
