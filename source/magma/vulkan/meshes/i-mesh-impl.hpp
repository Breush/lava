#pragma once

#include <lava/magma/meshes/i-mesh.hpp>

#include "../../bounding-sphere.hpp"

namespace lava::magma {
    /**
     * Interface for meshes.
     */
    class IMesh::Impl {
    public:
        virtual ~Impl() = default;

        virtual void init() = 0;

        /// Render the mesh.
        virtual void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex,
                            uint32_t materialDescriptorSetIndex) = 0;

        /// The bounding sphere of the mesh vertices (world-space).
        virtual const BoundingSphere& boundingSphere() const = 0;
    };
}
