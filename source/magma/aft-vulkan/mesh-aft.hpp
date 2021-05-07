#pragma once

#include "../vulkan/holders/acceleration-structure-holder.hpp"
#include "../vulkan/holders/buffer-holder.hpp"

namespace lava::magma {
    class Mesh;
    class Scene;
}

namespace lava::magma {
    class MeshAft {
    public:
        MeshAft(Mesh& fore, Scene& scene);

        void update();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                    uint32_t materialDescriptorSetIndex) const;
        void renderUnlit(vk::CommandBuffer commandBuffer) const;

        const vulkan::BufferHolder& indexBufferHolder() const { return m_indexBufferHolder; }
        const vulkan::BufferHolder& vertexBufferHolder() const { return m_vertexBufferHolder; }
        const vulkan::BufferHolder& unlitVertexBufferHolder() const { return m_unlitVertexBufferHolder; }
        const vulkan::BufferHolder& instanceBufferHolder() const { return m_instanceBufferHolder; }

        const vulkan::AccelerationStructureHolder& blasHolder() const { return m_blasHolder; }

        // ----- Fore
        void foreVerticesChanged() { m_vertexBufferDirty = true; }
        void foreInstancesCountChanged() { m_instanceBufferDirty = true; }
        void foreUboChanged(uint32_t /* instanceIndex */) { m_instanceBufferDirty = true; }
        void foreIndicesChanged();

    protected:
        void createVertexBuffers();
        void createInstanceBuffer();
        void createIndexBuffer();
        void createBlas();

    private:
        Mesh& m_fore;
        Scene& m_scene;

        // ----- Geometry
        vulkan::AccelerationStructureHolder m_blasHolder;
        vulkan::BufferHolder m_blasTransformBufferHolder;
        vulkan::BufferHolder m_unlitVertexBufferHolder;
        vulkan::BufferHolder m_vertexBufferHolder;
        vulkan::BufferHolder m_instanceBufferHolder;
        vulkan::BufferHolder m_indexBufferHolder;
        bool m_vertexBufferDirty = false;
        bool m_instanceBufferDirty = false;
    };
}
