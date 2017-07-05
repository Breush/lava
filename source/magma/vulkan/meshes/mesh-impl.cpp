#include "./mesh-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <lava/chamber/logger.hpp>
#include <lava/magma/materials/rm-material.hpp>

#include "../buffer.hpp"
#include "../render-engine-impl.hpp"

using namespace lava::chamber;
using namespace lava::magma;

Mesh::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
    , m_device(m_engine.device())
    , m_uniformStagingBuffer({m_device.capsule(), vkDestroyBuffer})
    , m_uniformStagingBufferMemory({m_device.capsule(), vkFreeMemory})
    , m_uniformBuffer({m_device.capsule(), vkDestroyBuffer})
    , m_uniformBufferMemory({m_device.capsule(), vkFreeMemory})
    , m_vertexBuffer({m_device.capsule(), vkDestroyBuffer})
    , m_vertexBufferMemory({m_device.capsule(), vkFreeMemory})
    , m_indexBuffer({m_device.capsule(), vkDestroyBuffer})
    , m_indexBufferMemory({m_device.capsule(), vkFreeMemory})
{
    // Create descriptor set
    VkDescriptorSetLayout layouts[] = {m_engine.meshDescriptorSetLayout()};
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_engine.meshDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
        logger.error("magma.vulkan.mesh") << "Failed to create descriptor set." << std::endl;
    }

    // Create uniform buffer
    VkDeviceSize bufferSize = sizeof(MeshUbo);

    int bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    int memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformStagingBuffer,
                         m_uniformStagingBufferMemory);

    bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformBuffer, m_uniformBufferMemory);

    // Set it up
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(MeshUbo);

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0u;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_device, 1u, &descriptorWrite, 0, nullptr);

    updateBindings();
}

Mesh::Impl::~Impl()
{
    vkDeviceWaitIdle(m_device);
}

void Mesh::Impl::positionAdd(const glm::vec3& delta)
{
    // @todo Should use local one and dirtify the world one
    m_worldTransform = glm::translate(m_worldTransform, delta);
    updateBindings();
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

void Mesh::Impl::verticesUvs(const std::vector<glm::vec2>& uvs)
{
    auto length = std::min(m_vertices.size(), uvs.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].uv = uvs[i];
    }

    createVertexBuffer();
}

void Mesh::Impl::verticesNormals(const std::vector<glm::vec3>& normals)
{
    auto length = std::min(m_vertices.size(), normals.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].normal = normals[i];
    }

    createVertexBuffer();
}

void Mesh::Impl::verticesTangents(const std::vector<glm::vec4>& tangents)
{
    auto length = std::min(m_vertices.size(), tangents.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].tangent = tangents[i];
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

void Mesh::Impl::material(RmMaterial& material)
{
    m_material = &material;
}

void Mesh::Impl::updateBindings()
{
    // Update UBOs
    MeshUbo meshUbo = {};
    meshUbo.transform = m_worldTransform;

    void* data;
    vkMapMemory(m_device, m_uniformStagingBufferMemory, 0, sizeof(MeshUbo), 0, &data);
    memcpy(data, &meshUbo, sizeof(MeshUbo));
    vkUnmapMemory(m_device, m_uniformStagingBufferMemory);

    vulkan::copyBuffer(m_device, m_engine.commandPool(), m_uniformStagingBuffer, m_uniformBuffer, sizeof(MeshUbo));
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

// ----- IMesh -----

IMesh::UserData Mesh::Impl::render(IMesh::UserData data)
{
    auto& commandBuffer = *reinterpret_cast<VkCommandBuffer*>(data);

    // Bind the material
    // @todo Have this in a more clever render loop, and not called by this mesh
    if (m_material != nullptr) m_material->render(data);

    // Bind with the mesh descriptor set
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_engine.pipelineLayout(), DESCRIPTOR_SET_INDEX, 1,
                            &m_descriptorSet, 0, nullptr);

    // Add the vertex buffer
    VkBuffer vertexBuffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // Draw
    vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);

    return nullptr;
}
