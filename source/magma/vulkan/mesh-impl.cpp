#include "./mesh-impl.hpp"

#include "./material-impl.hpp"
#include "./render-scenes/render-scene-impl.hpp"
#include "./ubos.hpp"

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
    m_descriptorSet = m_scene.meshDescriptorHolder().allocateSet("mesh");
    m_uboHolder.init(m_descriptorSet, m_scene.meshDescriptorHolder().uniformBufferBindingOffset(), {sizeof(vulkan::MeshUbo)});

    m_initialized = true;
    updateBindings();
}

// ----- Transform

void Mesh::Impl::transform(const glm::mat4& transform)
{
    m_transform = transform;
    // @todo Should decompose translation/rotation/scaling
    updateBoundingSphere();
    updateBindings();
}

void Mesh::Impl::translate(const glm::vec3& delta)
{
    m_translation += delta;
    updateTransform();
    updateBoundingSphere();
    updateBindings();
}

void Mesh::Impl::rotate(const glm::vec3& axis, float angleDelta)
{
    m_rotation = glm::rotate(m_rotation, angleDelta, axis);
    updateTransform();
    updateBoundingSphere();
    updateBindings();
}

void Mesh::Impl::scale(float factor)
{
    m_scaling *= factor;
    updateTransform();
    updateBoundingSphere();
    updateBindings();
}

// ----- Geometry

void Mesh::Impl::verticesCount(const uint32_t count)
{
    m_vertices.resize(count);
}

void Mesh::Impl::verticesPositions(VectorView<glm::vec3> positions)
{
    // @note We compute the center of the bounding sphere as the average
    // of all vertices. That is surely not perfect, but that's some heuristic so far.
    m_boundingSphereLocal.center = glm::vec3(0.f);
    for (const auto& position : positions) {
        m_boundingSphereLocal.center += position;
    }
    m_boundingSphereLocal.center /= positions.size();

    auto maxDistanceSquared = 0.f;
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), positions.size());
    for (uint32_t i = 0u; i < length; ++i) {
        // Storing the position.
        const auto& position = positions[i];
        m_vertices[i].pos = position;

        // Finding the bounding sphere radius.
        auto vertexVector = position - m_boundingSphereLocal.center;
        maxDistanceSquared = glm::dot(vertexVector, vertexVector);
    }

    m_boundingSphereLocal.radius = std::sqrt(maxDistanceSquared);

    updateBoundingSphere();
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

void Mesh::Impl::updateTransform()
{
    m_transform = glm::mat4();
    m_transform = glm::scale(m_transform, m_scaling);
    m_transform = glm::mat4_cast(m_rotation) * m_transform;
    // @note glm::translate wrongly takes scaling into account
    m_transform[3] = glm::vec4(m_translation, 1.f);
}

void Mesh::Impl::updateBoundingSphere()
{
    // @note The bounding radius can be overestimated if
    // the three scaling components are not the same,
    // but this is way faster than reevaluating from all transformed
    // vertices.
    // auto scalingX = glm::length(m_transform[0]);
    // auto scalingY = glm::length(m_transform[1]);
    // auto scalingZ = glm::length(m_transform[2]);
    auto maxScaling = std::max(std::max(m_scaling[0], m_scaling[1]), m_scaling[2]);

    m_boundingSphere.center = glm::vec3(m_transform * glm::vec4(m_boundingSphereLocal.center, 1));
    m_boundingSphere.radius = maxScaling * m_boundingSphereLocal.radius;
}

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

// ----- Internal interface -----

void Mesh::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex,
                        uint32_t materialDescriptorSetIndex)
{
    if (m_vertices.empty() || m_indices.empty()) return;

    // Bind the material
    // @todo Have this in a more clever render loop, and not called by this mesh
    // Fact is we shouldn't know about the correct descriptorSetIndex here
    if (materialDescriptorSetIndex != -1u) {
        if (m_material != nullptr) {
            m_material->impl().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
        }
        else {
            m_scene.fallbackMaterial().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
        }
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
