#pragma once

#include <lava/chamber/properties.hpp>
#include <lava/magma/engine.hpp>
#include <lava/magma/mesh.hpp>

#include "./capsule.hpp"
#include "./device.hpp"
#include "./vertex.hpp"

namespace lava {
    /**
     * Vulkan-based implementation of a mesh.
     */
    class Mesh::Impl {
    public:
        Impl(Engine& engine);
        ~Impl();

        // Main interface
        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec2>& positions);
        void verticesColors(const std::vector<glm::vec3>& colors);
        void indices(const std::vector<uint16_t>& indices);

        // Internal interface
        void update();
        void addCommands(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        // References
        Engine::Impl& m_engine;
        vulkan::Device& m_device;

        // Data
        std::vector<vulkan::Vertex> m_vertices;
        std::vector<uint16_t> m_indices;

        // Buffers
        vulkan::Capsule<VkBuffer> m_vertexBuffer;
        vulkan::Capsule<VkDeviceMemory> m_vertexBufferMemory;
        vulkan::Capsule<VkBuffer> m_indexBuffer;
        vulkan::Capsule<VkDeviceMemory> m_indexBufferMemory;
    };
}
