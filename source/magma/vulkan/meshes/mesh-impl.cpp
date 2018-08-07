#include "./mesh-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <lava/chamber/logger.hpp>

#include "../material-impl.hpp"
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

Mesh::Impl::~Impl()
{
    if (m_initialized) {
        m_scene.meshDescriptorHolder().freeSet(m_descriptorSet);
    }
}

void Mesh::Impl::init()
{
    m_descriptorSet = m_scene.meshDescriptorHolder().allocateSet();
    m_uboHolder.init(m_descriptorSet, m_scene.meshDescriptorHolder().uniformBufferBindingOffset(), {sizeof(vulkan::MeshUbo)});

    m_initialized = true;
    updateBindings();
}

// ----- Transform

void Mesh::Impl::transform(const glm::mat4& transform)
{
    m_transform = transform;
    updateBindings();
}

void Mesh::Impl::translate(const glm::vec3& delta)
{
    // @todo Should use local one and dirtify the world one
    m_transform = glm::translate(m_transform, delta);
    updateBindings();
}

void Mesh::Impl::rotate(const glm::vec3& axis, float angleDelta)
{
    m_transform = glm::rotate(m_transform, angleDelta, axis);
    updateBindings();
}

// ----- Geometry

void Mesh::Impl::verticesCount(const uint32_t count)
{
    m_vertices.resize(count);
}

void Mesh::Impl::verticesPositions(VectorView<glm::vec3> positions)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), positions.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].pos = positions[i];
    }

    createVertexBuffer();
}

void Mesh::Impl::verticesUvs(VectorView<glm::vec2> uvs)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), uvs.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].uv = uvs[i];
    }

    createVertexBuffer();
}

void Mesh::Impl::verticesNormals(VectorView<glm::vec3> normals)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), normals.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].normal = normals[i];
    }

    createVertexBuffer();
}

void Mesh::Impl::verticesTangents(VectorView<glm::vec4> tangents)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), tangents.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].tangent = glm::vec4(glm::vec3(tangents[i]), tangents[i].w);
    }

    createVertexBuffer();
}

void Mesh::Impl::indices(VectorView<uint16_t> indices)
{
    auto length = indices.size();
    m_indices.resize(length);
    for (auto i = 0u; i < length; ++i) {
        m_indices[i] = indices[i];
    }
    createIndexBuffer();

    // @todo The scene should update the main command buffer every frame
    // while we update our secondary command buffer right here
}

// ----- Material

void Mesh::Impl::material(Material& material)
{
    m_material = &material;
}

// ----- Internal

void Mesh::Impl::updateBindings()
{
    if (!m_initialized) return;

    vulkan::MeshUbo ubo;
    ubo.transform = m_transform;
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
    if (m_indices.empty()) {
        logger.warning("magma.vulkan.mesh") << "No indices provided. The mesh will not be visible." << std::endl;
        return;
    }

    vk::DeviceSize bufferSize = sizeof(uint16_t) * m_indices.size();

    m_indexBufferHolder.create(vk::BufferUsageFlagBits::eIndexBuffer, bufferSize);
    m_indexBufferHolder.copy(m_indices.data(), bufferSize);
}

// ----- IMesh -----

void Mesh::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex,
                        uint32_t materialDescriptorSetIndex)
{
    if (m_vertices.empty() || m_indices.empty()) return;

    // Bind the material
    // @todo Have this in a more clever render loop, and not called by this mesh
    // Fact is we shouldn't know about the correct descriptorSetIndex here
    if (m_material != nullptr) {
        m_material->impl().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
    }
    else {
        m_scene.fallbackMaterial().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
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
