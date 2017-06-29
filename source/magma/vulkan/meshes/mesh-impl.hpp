#pragma once

#include <lava/chamber/properties.hpp>
#include <lava/magma/meshes/mesh.hpp>
#include <lava/magma/render-engine.hpp>

#include "../capsule.hpp"
#include "../device.hpp"
#include "../vertex.hpp"

namespace lava {
    /**
     * Vulkan-based implementation of a mesh.
     */
    class Mesh::Impl {
    public:
        Impl(RenderEngine& engine);
        ~Impl();

        // IMesh
        // void update();
        void* render(void* data);

        // Main interface
        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesNormals(const std::vector<glm::vec3>& normals);
        void verticesTangents(const std::vector<glm::vec4>& tangents);
        void verticesColors(const std::vector<glm::vec3>& colors);
        void verticesColors(const glm::vec3& color);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void indices(const std::vector<uint16_t>& indices);
        void material(const MrrMaterial& material);

    private:
        void createDescriptorSet();
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        // References
        RenderEngine::Impl& m_engine;
        vulkan::Device& m_device;

        // Data
        std::vector<vulkan::Vertex> m_vertices;
        std::vector<uint16_t> m_indices;
        const MrrMaterial* m_material = nullptr;

        // Buffers
        vulkan::Capsule<VkBuffer> m_vertexBuffer;
        vulkan::Capsule<VkDeviceMemory> m_vertexBufferMemory;
        vulkan::Capsule<VkBuffer> m_indexBuffer;
        vulkan::Capsule<VkDeviceMemory> m_indexBufferMemory;
    };
}
