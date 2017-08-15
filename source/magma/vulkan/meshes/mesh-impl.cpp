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
    , m_vertexBufferHolder(m_engine)
    , m_indexBufferHolder(m_engine)
{
    // Create descriptor set
    m_descriptorSet = m_engine.meshDescriptorHolder().allocateSet();

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

    m_vertexBufferHolder.create(vk::BufferUsageFlagBits::eVertexBuffer, bufferSize);
    m_vertexBufferHolder.copy(m_vertices.data(), bufferSize);
}

void Mesh::Impl::createIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(uint16_t) * m_indices.size();

    m_indexBufferHolder.create(vk::BufferUsageFlagBits::eIndexBuffer, bufferSize);
    m_indexBufferHolder.copy(m_indices.data(), bufferSize);
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
    commandBuffer.bindVertexBuffers(0, 1, &m_vertexBufferHolder.buffer(), offsets);
    commandBuffer.bindIndexBuffer(m_indexBufferHolder.buffer(), 0, vk::IndexType::eUint16);

    // Draw
    commandBuffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);

    return nullptr;
}
