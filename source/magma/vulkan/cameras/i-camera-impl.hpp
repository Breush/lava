#pragma once

#include <lava/magma/cameras/i-camera.hpp>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace lava::magma {
    /**
     * Interface for cameras.
     */
    class ICamera::Impl {
    public:
        virtual ~Impl() = default;

    public:
        virtual void init() = 0;

        /// Render the camera.
        virtual void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) = 0;

        /// Its world position.
        virtual const glm::vec3& position() const = 0;

        /// Its view transform.
        virtual const glm::mat4& viewTransform() const = 0;

        /// Its projection transform.
        virtual const glm::mat4& projectionTransform() const = 0;
    };
}
