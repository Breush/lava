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
    , m_uniformStagingBuffer({m_device.capsule(), vkDestroyBuffer})
    , m_uniformStagingBufferMemory({m_device.capsule(), vkFreeMemory})
    , m_uniformBuffer({m_device.capsule(), vkDestroyBuffer})
    , m_uniformBufferMemory({m_device.capsule(), vkFreeMemory})
{
    m_engine.add(*this);

    createUniformBuffer();
}

Mesh::Impl::~Impl()
{
    vkDeviceWaitIdle(m_device);
}

void Mesh::Impl::vertices(const std::vector<glm::vec2>& vertices)
{
    m_vertices.resize(vertices.size());
    for (uint32_t i = 0u; i < vertices.size(); ++i) {
        m_vertices[i].pos = vertices[i];
        m_vertices[i].color = glm::vec3(1.f, 1.f, 0.f);
    }

    createVertexBuffer();
}

void Mesh::Impl::indices(const std::vector<uint16_t>& indices)
{
    m_indices = indices;
    createIndexBuffer();
}

void Mesh::Impl::update()
{
    // @todo Get time!
    static float time = 0.f;
    const float dt = 0.001f;
    const auto viewExtent = m_engine.swapchain().extent();

    time += dt;

    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f), viewExtent.width / (float)viewExtent.height, 0.1f, 10.0f);
    ubo.projection[1][1] *= -1; // Well, for this is not OpenGL!

    void* data;
    vkMapMemory(m_device, m_uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_device, m_uniformStagingBufferMemory);

    vulkan::copyBuffer(m_device, m_engine.commandPool(), m_uniformStagingBuffer, m_uniformBuffer, sizeof(ubo));
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

void Mesh::Impl::createUniformBuffer()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    int bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    int memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformStagingBuffer,
                         m_uniformStagingBufferMemory);

    bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformBuffer, m_uniformBufferMemory);
}

void Mesh::Impl::addCommands(VkCommandBuffer commandBuffer)
{
    // Add the vertex buffer
    VkBuffer vertexBuffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // Add uniform buffers
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_engine.pipelineLayout(), 0, 1,
                            &m_engine.descriptorSet(), 0, nullptr);

    // Draw
    vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
}
