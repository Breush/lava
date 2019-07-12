#pragma once

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../ubos.hpp"
#include "../vulkan/holders/buffer-holder.hpp"

namespace lava::magma {
    class Mesh;
}

namespace lava::magma {
    class MeshAft {
    public:
        MeshAft(Mesh& fore, RenderScene::Impl& scene);

        void init();
        void update();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset,
                    uint32_t materialDescriptorSetIndex) const;
        void renderUnlit(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset) const;

        // ----- Fore
        void foreTransformChanged();
        void foreVerticesChanged() { m_vertexBufferDirty = true; }
        void foreIndicesChanged();

    protected:
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        Mesh& m_fore;
        RenderScene::Impl& m_scene;

        // ----- Shader data
        // @todo PUT that in Mesh because it's cross-tech?
        MeshUbo m_ubo;

        // ----- Geometry
        vulkan::BufferHolder m_unlitVertexBufferHolder;
        vulkan::BufferHolder m_vertexBufferHolder;
        vulkan::BufferHolder m_indexBufferHolder;
        bool m_vertexBufferDirty = false;
    };
    constexpr auto U = sizeof(MeshAft);
}
