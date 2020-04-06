#pragma once

#include "../vulkan/holders/buffer-holder.hpp"

namespace lava::magma {
    class Flat;
    class Scene;
}

namespace lava::magma {
    class FlatAft {
    public:
        FlatAft(Flat& fore, Scene& scene);

        void update();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset,
                    uint32_t materialDescriptorSetIndex) const;

        // ----- Fore
        void foreVerticesChanged() { m_vertexBufferDirty = true; }
        void foreIndicesChanged();

    protected:
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        Flat& m_fore;
        Scene& m_scene;

        // ----- Geometry
        vulkan::BufferHolder m_vertexBufferHolder;
        vulkan::BufferHolder m_indexBufferHolder;
        bool m_vertexBufferDirty = false;
    };
}
