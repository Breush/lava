#pragma once

#include <lava/chamber/macros.hpp>
#include <lava/magma/meshes/mesh.hpp>
#include <lava/magma/render-engine.hpp>

#include "../capsule.hpp"
#include "../device.hpp"
#include "../vertex.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of a mesh.
     */
    class Mesh::Impl {
        struct MeshUbo {
            glm::mat4 transform;
        };

    public:
        Impl(RenderEngine& engine);
        ~Impl();

        // IMesh
        const glm::mat4& worldTransform() const { return m_worldTransform; }
        IMesh::UserData render(IMesh::UserData data);

        // Mesh
        void positionAdd(const glm::vec3& delta);

        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesNormals(const std::vector<glm::vec3>& normals);
        void verticesTangents(const std::vector<glm::vec4>& tangents);
        void verticesColors(const std::vector<glm::vec3>& colors);
        void verticesColors(const glm::vec3& color);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void indices(const std::vector<uint16_t>& indices);

        RmMaterial& material() { return *m_material; }
        void material(RmMaterial& material);

    private:
        void updateBindings();
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        // References
        RenderEngine::Impl& m_engine;
        vulkan::Device& m_device;

        // Data
        std::vector<vulkan::Vertex> m_vertices;
        std::vector<uint16_t> m_indices;
        RmMaterial* m_material = nullptr;

        // Node
        glm::mat4 m_worldTransform;

        // Descriptor
        VkDescriptorSet m_descriptorSet;
        vulkan::Capsule<VkBuffer> m_uniformStagingBuffer;
        vulkan::Capsule<VkDeviceMemory> m_uniformStagingBufferMemory;
        vulkan::Capsule<VkBuffer> m_uniformBuffer;
        vulkan::Capsule<VkDeviceMemory> m_uniformBufferMemory;

        // Buffers
        vulkan::Capsule<VkBuffer> m_vertexBuffer;
        vulkan::Capsule<VkDeviceMemory> m_vertexBufferMemory;
        vulkan::Capsule<VkBuffer> m_indexBuffer;
        vulkan::Capsule<VkDeviceMemory> m_indexBufferMemory;
    };
}
