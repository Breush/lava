#include "./mesh-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <lava/chamber/logger.hpp>

#include "./buffer.hpp"
#include "./engine-impl.hpp"

using namespace lava;

Mesh::Impl::Impl(Engine& engine)
    : m_engine(engine.impl())
    , m_device(m_engine.device())
    , m_vertexBuffer({m_device.capsule(), vkDestroyBuffer})
    , m_vertexBufferMemory({m_device.capsule(), vkFreeMemory})
    , m_indexBuffer({m_device.capsule(), vkDestroyBuffer})
    , m_indexBufferMemory({m_device.capsule(), vkFreeMemory})
{
    m_engine.add(*this);
}

Mesh::Impl::~Impl()
{
    vkDeviceWaitIdle(m_device);
}

void Mesh::Impl::verticesCount(const uint32_t count)
{
    m_vertices.resize(count);
}

void Mesh::Impl::verticesPositions(const std::vector<glm::vec3>& positions)
{
    auto length = std::min(m_vertices.size(), positions.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].pos = positions[i];
    }

    createVertexBuffer();
}

void Mesh::Impl::verticesColors(const std::vector<glm::vec3>& colors)
{
    auto length = std::min(m_vertices.size(), colors.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].color = colors[i];
    }

    createVertexBuffer();
}

void Mesh::Impl::indices(const std::vector<uint16_t>& indices)
{
    m_indices = indices;
    createIndexBuffer();

    // @todo The engine should update the main command buffer every frame
    // while we update our secondary command buffer right here
}

void Mesh::Impl::update()
{
    // UBOs ?
}

void Mesh::Impl::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vulkan::Vertex) * m_vertices.size();

    // Staging buffer
    vulkan::Capsule<VkBuffer> stagingBuffer{m_device.capsule(), vkDestroyBuffer};
    vulkan::Capsule<VkDeviceMemory> stagingBufferMemory{m_device.capsule(), vkFreeMemory};
    int bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    int memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    // Actual vertex buffer
    bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_vertexBuffer, m_vertexBufferMemory);

    // Copy
    vulkan::copyBuffer(m_device, m_engine.commandPool(), stagingBuffer, m_vertexBuffer, bufferSize);
}

void Mesh::Impl::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(uint16_t) * m_indices.size();

    // Staging buffer
    vulkan::Capsule<VkBuffer> stagingBuffer{m_device.capsule(), vkDestroyBuffer};
    vulkan::Capsule<VkDeviceMemory> stagingBufferMemory{m_device.capsule(), vkFreeMemory};
    int bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    int memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    // Actual index buffer
    bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_indexBuffer, m_indexBufferMemory);

    // Copy
    vulkan::copyBuffer(m_device, m_engine.commandPool(), stagingBuffer, m_indexBuffer, bufferSize);
}

void Mesh::Impl::addCommands(VkCommandBuffer commandBuffer)
{
    // Add the vertex buffer
    VkBuffer vertexBuffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // Draw
    vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
}
