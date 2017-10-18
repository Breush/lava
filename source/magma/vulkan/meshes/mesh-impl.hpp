#pragma once

#include <lava/magma/meshes/mesh.hpp>

#include "./i-mesh-impl.hpp"

#include <lava/chamber/macros.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/buffer-holder.hpp"
#include "../holders/ubo-holder.hpp"
#include "../vertex.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of a mesh.
     */
    class Mesh::Impl final : public IMesh::Impl {
    public:
        Impl(RenderScene& scene);

        // IMesh
        bool translucent() const { return m_translucent; }
        void translucent(bool translucent) { m_translucent = translucent; }

        // IMesh::Impl
        void init() override final;
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                    uint32_t descriptorSetIndex) override final;

        // Mesh
        const glm::mat4& transform() const { return m_transform; }
        void transform(const glm::mat4& transform);
        void positionAdd(const glm::vec3& delta);
        void rotationAdd(const glm::vec3& axis, float angleDelta);

        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void verticesNormals(const std::vector<glm::vec3>& normals);
        void verticesTangents(const std::vector<glm::vec4>& tangents);
        void indices(const std::vector<uint16_t>& indices);

        Material& material() { return *m_material; }
        void material(Material& material);

    private:
        void updateBindings();
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        // References
        RenderScene::Impl& m_scene;
        bool m_initialized = false;

        // Data
        bool m_translucent = false;
        std::vector<vulkan::Vertex> m_vertices;
        std::vector<uint16_t> m_indices;
        Material* m_material = nullptr;

        // Node
        glm::mat4 m_transform;

        // Descriptor
        vk::DescriptorSet m_descriptorSet;
        vulkan::UboHolder m_uboHolder;

        // Vertices
        vulkan::BufferHolder m_vertexBufferHolder;
        vulkan::BufferHolder m_indexBufferHolder;
    };
}
