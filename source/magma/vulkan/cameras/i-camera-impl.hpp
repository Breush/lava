#pragma once

#include <lava/magma/cameras/i-camera.hpp>

#include "../../frustum.hpp"

namespace lava::magma {
    /**
     * Interface for cameras.
     */
    class ICamera::Impl {
    public:
        virtual ~Impl() = default;

    public:
        /// Whether the camera is used to render in a VR render target.
        virtual bool vrAimed() const = 0;

        //----- ICamera

        /// Initialize when all references are ready.
        virtual void init(uint32_t id) = 0;

        /// Render the camera.
        virtual void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                            uint32_t pushConstantOffset) const = 0;

        /// The rendered extent.
        virtual vk::Extent2D renderExtent() const = 0;

        /// Its world translation.
        virtual const glm::vec3& translation() const = 0;

        /**
         * Its view transform.
         *
         * For the camera, the convention for this specific transform is:
         *      -Z forward
         *      +Y up
         *      +X right (right-handed)
         */
        virtual const glm::mat4& viewTransform() const = 0;

        /// Its projection transform.
        virtual const glm::mat4& projectionTransform() const = 0;

        /// Whether frustum culling is enabled.
        virtual bool useFrustumCulling() const = 0;

        /// Its frustum.
        virtual const Frustum& frustum() const = 0;

        virtual float nearClip() const = 0;
        virtual void nearClip(float nearClip) = 0;
        virtual float farClip() const = 0;
        virtual void farClip(float farClip) = 0;
    };
}
