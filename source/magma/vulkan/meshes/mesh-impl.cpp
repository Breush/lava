#include "./mesh-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <lava/chamber/logger.hpp>
#include <lava/magma/interfaces/material.hpp>

#include "../buffer.hpp"
#include "../render-engine-impl.hpp"
#include "../user-data-render.hpp"

using namespace lava::chamber;
using namespace lava::magma;

Mesh::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
    , m_uniformBufferHolder(m_engine)
    , m_vertexBuffer{m_engine.device()}
    , m_vertexBufferMemory{m_engine.device()}
    , m_indexBuffer{m_engine.device()}
    , m_indexBufferMemory{m_engine.device()}
{
    // Create descriptor set
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.descriptorPool = m_engine.meshDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_engine.meshDescriptorSetLayout();

    if (m_engine.device().allocateDescriptorSets(&allocInfo, &m_descriptorSet) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.mesh") << "Failed to create descriptor set." << std::endl;
    }

    // Create uniform buffer
    m_uniformBufferHolder.create(vk::BufferUsageFlagBits::eUniformBuffer, sizeof(MeshUbo));

    // Set it up
    // @todo There is a lot a common code between Mesh::Impl, OrbitCamera::Impl and RmMaterial::Impl
    // about descriptor sets. Find a way to refacto.
    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = m_uniformBufferHolder.buffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(MeshUbo);

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0u;
    descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);

    updateBindings();
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

void Mesh::Impl::material(IMaterial& material)
{
    m_material = &material;
}

void Mesh::Impl::updateBindings()
{
    MeshUbo ubo = {};
    ubo.transform = m_worldTransform;
    m_uniformBufferHolder.copy(ubo);
}

void Mesh::Impl::createVertexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(vulkan::Vertex) * m_vertices.size();

    // Staging buffer
    vulkan::Buffer stagingBuffer{m_engine.device()};
    vulkan::DeviceMemory stagingBufferMemory{m_engine.device()};
    vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    vk::MemoryPropertyFlags memoryPropertyFlags =
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    vulkan::createBuffer(m_engine.device(), m_engine.physicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags,
                         stagingBuffer, stagingBufferMemory);

    void* data;
    m_engine.device().mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &data);
    memcpy(data, m_vertices.data(), (size_t)bufferSize);
    m_engine.device().unmapMemory(stagingBufferMemory);

    // Actual vertex buffer
    bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    vulkan::createBuffer(m_engine.device(), m_engine.physicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags,
                         m_vertexBuffer, m_vertexBufferMemory);

    // Copy
    vulkan::copyBuffer(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), stagingBuffer, m_vertexBuffer,
                       bufferSize);
}

void Mesh::Impl::createIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(uint16_t) * m_indices.size();

    // Staging buffer
    vulkan::Buffer stagingBuffer{m_engine.device()};
    vulkan::DeviceMemory stagingBufferMemory{m_engine.device()};
    vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    vk::MemoryPropertyFlags memoryPropertyFlags =
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    vulkan::createBuffer(m_engine.device(), m_engine.physicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags,
                         stagingBuffer, stagingBufferMemory);

    void* data;
    m_engine.device().mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &data);
    memcpy(data, m_indices.data(), (size_t)bufferSize);
    m_engine.device().unmapMemory(stagingBufferMemory);

    // Actual index buffer
    bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    vulkan::createBuffer(m_engine.device(), m_engine.physicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags,
                         m_indexBuffer, m_indexBufferMemory);

    // Copy
    vulkan::copyBuffer(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), stagingBuffer, m_indexBuffer,
                       bufferSize);
}

// ----- IMesh -----

IMesh::UserData Mesh::Impl::render(IMesh::UserData data)
{
    auto& userData = *reinterpret_cast<UserDataRenderIn*>(data);
    const auto& commandBuffer = *userData.commandBuffer;
    const auto& pipelineLayout = *userData.pipelineLayout;

    // Bind the material
    // @todo Have this in a more clever render loop, and not called by this mesh
    if (m_material != nullptr) m_material->render(data);

    // Bind with the mesh descriptor set
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, DESCRIPTOR_SET_INDEX, 1, &m_descriptorSet,
                                     0, nullptr);

    // Add the vertex buffer
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, &m_vertexBuffer, offsets);
    commandBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint16);

    // Draw
    commandBuffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);

    return nullptr;
}
