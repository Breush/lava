#pragma once

#include <lava/magma/ubos.hpp>

#include "../vulkan/holders/buffer-holder.hpp"

namespace lava::magma {
    class Mesh;
    class Scene;
}

namespace lava::magma {
    class MeshAft {
    public:
        MeshAft(Mesh& fore, Scene& scene);

        void init();
        void update();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset,
                    uint32_t materialDescriptorSetIndex) const;
        void renderUnlit(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset) const;

        // ----- Fore
        void foreVerticesChanged() { m_vertexBufferDirty = true; }
        void foreIndicesChanged();

    protected:
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        Mesh& m_fore;
        Scene& m_scene;

        // ----- Geometry
        vulkan::BufferHolder m_unlitVertexBufferHolder;
        vulkan::BufferHolder m_vertexBufferHolder;
        vulkan::BufferHolder m_indexBufferHolder;
        bool m_vertexBufferDirty = false;
    };
}
