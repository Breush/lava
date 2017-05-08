#pragma once

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

        // Main interface

        void vertices(const std::vector<glm::vec2>& vertices);
        void indices(const std::vector<uint16_t>& indices);

        // Internal interface

        void update();
        void addCommands(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffer();

        void createDescriptorSet();

        void createCommandBuffers();

    private:
        // References
        Engine::Impl& m_engine;
        vulkan::Device& m_device;

        // Data
        std::vector<vulkan::Vertex> m_vertices;
        std::vector<uint16_t> m_indices;

        // Buffers
        std::vector<VkCommandBuffer> m_commandBuffers;

        vulkan::Capsule<VkBuffer> m_vertexBuffer;
        vulkan::Capsule<VkDeviceMemory> m_vertexBufferMemory;
        vulkan::Capsule<VkBuffer> m_indexBuffer;
        vulkan::Capsule<VkDeviceMemory> m_indexBufferMemory;

        vulkan::Capsule<VkBuffer> m_uniformStagingBuffer;
        vulkan::Capsule<VkDeviceMemory> m_uniformStagingBufferMemory;
        vulkan::Capsule<VkBuffer> m_uniformBuffer;
        vulkan::Capsule<VkDeviceMemory> m_uniformBufferMemory;

        VkDescriptorSet m_descriptorSet;
    };
}
