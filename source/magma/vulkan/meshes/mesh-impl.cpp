#include "./mesh-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <lava/chamber/logger.hpp>

#include "../materials/i-material-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../ubos.hpp"

using namespace lava::chamber;
using namespace lava::magma;

Mesh::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
    , m_vertexBufferHolder(m_scene.engine())
    , m_indexBufferHolder(m_scene.engine())
{
}

void Mesh::Impl::init()
{
    m_descriptorSet = m_scene.meshDescriptorHolder().allocateSet();
    m_uboHolder.init(m_descriptorSet, m_scene.meshDescriptorHolder().uniformBufferBindingOffset(), {sizeof(vulkan::MeshUbo)});

    m_initialized = true;
    updateBindings();
}

void Mesh::Impl::positionAdd(const glm::vec3& delta)
{
    // @todo Should use local one and dirtify the world one
    m_worldTransform = glm::translate(m_worldTransform, delta);
    updateBindings();
}

void Mesh::Impl::rotationAdd(const glm::vec3& axis, float angleDelta)
{
    m_worldTransform = glm::rotate(m_worldTransform, angleDelta, axis);
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

    // @todo The scene should update the main command buffer every frame
    // while we update our secondary command buffer right here
}

void Mesh::Impl::material(IMaterial& material)
{
    m_material = &material;
}

void Mesh::Impl::updateBindings()
{
    if (!m_initialized) return;

    vulkan::MeshUbo ubo;
    ubo.transform = m_worldTransform;
    m_uboHolder.copy(0, ubo);
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

void Mesh::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex)
{
    // Bind the material
    // @todo Have this in a more clever render loop, and not called by this mesh
    // Fact is we shouldn't know about the correct descriptorSetIndex here
    if (m_material != nullptr) {
        m_material->interfaceImpl().render(commandBuffer, pipelineLayout, 1u);
    }
    else {
        m_scene.fallbackMaterial().render(commandBuffer, pipelineLayout, 1u);
    }

    // Bind with the mesh descriptor set
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);

    // Add the vertex buffer
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, &m_vertexBufferHolder.buffer(), offsets);
    commandBuffer.bindIndexBuffer(m_indexBufferHolder.buffer(), 0, vk::IndexType::eUint16);

    // Draw
    commandBuffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);
}
