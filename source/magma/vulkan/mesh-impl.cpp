#include "./mesh-impl.hpp"

#include <lava/chamber/mikktspace.hpp>

#include "../meshes/mesh-tools.hpp"
#include "./material-impl.hpp"
#include "./render-scenes/render-scene-impl.hpp"

using namespace lava;
using namespace lava::chamber;
using namespace lava::magma;

namespace {
    template <class UInt>
    inline void setIndices(std::vector<uint16_t>& targetIndices, const VectorView<UInt>& indices, bool flipTriangles,
                           uint32_t verticesCount)
    {
        auto length = indices.size();
        targetIndices.resize(length);

        // Fast coherency check
        if (indices[0] >= verticesCount || indices[length / 2] >= verticesCount) {
            logger.warning("magma.vulkan.mesh") << "Vertices count: " << verticesCount << std::endl;
            logger.warning("magma.vulkan.mesh")
                << "Wrong vertex index: " << static_cast<uint16_t>(std::max(indices[0], indices[length / 2])) << std::endl;
            logger.error("magma.vulkan.mesh")
                << "Some indices are referencing indices bigger than the vertices count." << std::endl;
        }

        if (flipTriangles) {
            for (auto i = 0u; i < length; i += 3) {
                targetIndices[i] = indices[i + 2];
                targetIndices[i + 1] = indices[i + 1];
                targetIndices[i + 2] = indices[i];
            }
        }
        else {
            for (auto i = 0u; i < length; ++i) {
                targetIndices[i] = indices[i];
            }
        }
    }
}

Mesh::Impl::Impl(RenderScene& scene)
    : m_baseScene(scene)
    , m_scene(scene.impl())
    , m_unlitVertexBufferHolder(m_scene.engine())
    , m_vertexBufferHolder(m_scene.engine())
    , m_indexBufferHolder(m_scene.engine())
{
}

Mesh::Impl::~Impl()
{
    if (m_boundingSphereVisible) {
        boundingSphereVisible(false);
    }
}

void Mesh::Impl::init()
{
    m_initialized = true;
    updateBindings();
}

void Mesh::Impl::update()
{
    if (m_vertexBufferDirty) {
        createVertexBuffer();
    }
}

void Mesh::Impl::computeFlatNormals()
{
    for (auto i = 0u; i < m_indices.size(); i += 3u) {
        auto i0 = m_indices[i];
        auto i1 = m_indices[i + 1];
        auto i2 = m_indices[i + 2];
        auto& v0 = m_vertices[i0];
        auto& v1 = m_vertices[i1];
        auto& v2 = m_vertices[i2];
        v0.normal = glm::normalize(glm::cross(v1.pos - v2.pos, v1.pos - v0.pos));
        v1.normal = v0.normal;
        v2.normal = v0.normal;
    }

    m_vertexBufferDirty = true;
}

void Mesh::Impl::computeTangents()
{
    SMikkTSpaceInterface tsInterface;
    tsInterface.m_getNumFaces = [](const SMikkTSpaceContext* pContext) -> int {
        const auto& self = *reinterpret_cast<const Mesh::Impl*>(pContext->m_pUserData);
        return self.m_indices.size() / 3;
    };
    tsInterface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* /* pContext */, const int /* iFace */) -> int { return 3; };
    tsInterface.m_getPosition = [](const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert) {
        const auto& self = *reinterpret_cast<const Mesh::Impl*>(pContext->m_pUserData);
        auto vertexIndex = self.m_indices[3 * iFace + iVert];
        fvPosOut[0] = self.m_vertices[vertexIndex].pos[0];
        fvPosOut[1] = self.m_vertices[vertexIndex].pos[1];
        fvPosOut[2] = self.m_vertices[vertexIndex].pos[2];
    };
    tsInterface.m_getNormal = [](const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert) {
        const auto& self = *reinterpret_cast<const Mesh::Impl*>(pContext->m_pUserData);
        auto vertexIndex = self.m_indices[3 * iFace + iVert];
        fvNormOut[0] = self.m_vertices[vertexIndex].normal[0];
        fvNormOut[1] = self.m_vertices[vertexIndex].normal[1];
        fvNormOut[2] = self.m_vertices[vertexIndex].normal[2];
    };
    tsInterface.m_getTexCoord = [](const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert) {
        const auto& self = *reinterpret_cast<const Mesh::Impl*>(pContext->m_pUserData);
        auto vertexIndex = self.m_indices[3 * iFace + iVert];
        fvTexcOut[0] = self.m_vertices[vertexIndex].uv[0];
        fvTexcOut[1] = self.m_vertices[vertexIndex].uv[1];
    };

    m_temporaryVertices.clear();
    m_temporaryVertices.reserve(m_vertices.size());
    tsInterface.m_setTSpaceBasic = [](const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign,
                                      const int iFace, const int iVert) {
        auto& self = *reinterpret_cast<Mesh::Impl*>(pContext->m_pUserData);
        auto vertexIndex = self.m_indices[3 * iFace + iVert];

        vulkan::Vertex v;
        v = self.m_vertices[vertexIndex];
        v.tangent[0] = fvTangent[0];
        v.tangent[1] = fvTangent[1];
        v.tangent[2] = fvTangent[2];
        v.tangent[3] = fSign;
        self.m_temporaryVertices.emplace_back(std::move(v));
    };
    tsInterface.m_setTSpace = nullptr;

    SMikkTSpaceContext tsContext;
    tsContext.m_pUserData = this;
    tsContext.m_pInterface = &tsInterface;

    if (!genTangSpaceDefault(&tsContext)) {
        logger.warning("magma.vulkan.mesh") << "Could not generate tangents." << std::endl;
        return;
    }

    // Regenerating indices and unlit vertices
    m_vertices = m_temporaryVertices;
    auto indicesCount = m_vertices.size();
    m_indices.resize(indicesCount);
    m_unlitVertices.resize(indicesCount);
    for (auto i = 0u; i < indicesCount; ++i) {
        m_indices[i] = i;
        m_unlitVertices[i].pos = m_vertices[i].pos;
    }

    m_vertexBufferDirty = true;
    createIndexBuffer();

    m_temporaryVertices.resize(0);
}

// ----- Transform

void Mesh::Impl::transform(const glm::mat4& transform)
{
    m_transform = transform;

    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(m_transform, m_scaling, m_rotation, m_translation, skew, perspective);

    updateBoundingSphere();
    updateBindings();
}

void Mesh::Impl::translation(const glm::vec3& translation)
{
    m_translation = translation;

    updateTransform();
    updateBoundingSphere();
    updateBindings();
}

void Mesh::Impl::rotation(const glm::quat& rotation)
{
    m_rotation = rotation;

    updateTransform();
    updateBoundingSphere();
    updateBindings();
}

void Mesh::Impl::rotate(const glm::vec3& axis, float angleDelta)
{
    rotation(glm::rotate(m_rotation, angleDelta, axis));
}

void Mesh::Impl::scaling(const glm::vec3& scaling)
{
    m_scaling = scaling;

    updateTransform();
    updateBoundingSphere();
    updateBindings();
}

// ----- Geometry

void Mesh::Impl::verticesCount(const uint32_t count)
{
    m_unlitVertices.resize(count);
    m_vertices.resize(count);
}

void Mesh::Impl::verticesPositions(VectorView<glm::vec3> positions)
{
    // @note We compute the center of the bounding sphere
    // as the middle of each axis range.
    glm::vec3 minRange = positions[0];
    glm::vec3 maxRange = minRange;
    for (const auto& position : positions) {
        if (position.x > maxRange.x) maxRange.x = position.x;
        if (position.x < minRange.x) minRange.x = position.x;
        if (position.y > maxRange.y) maxRange.y = position.y;
        if (position.y < minRange.y) minRange.y = position.y;
        if (position.z > maxRange.z) maxRange.z = position.z;
        if (position.z < minRange.z) minRange.z = position.z;
    }
    m_boundingSphereLocal.center = (minRange + maxRange) / 2.f;
    m_boundingBoxExtentLocal = maxRange - minRange;

    auto maxDistanceSquared = 0.f;
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), positions.size());
    for (uint32_t i = 0u; i < length; ++i) {
        // Storing the position.
        const auto& position = positions[i];
        m_vertices[i].pos = position;
        m_unlitVertices[i].pos = position;

        // Finding the bounding sphere radius.
        auto vertexVector = position - m_boundingSphereLocal.center;
        maxDistanceSquared = glm::dot(vertexVector, vertexVector);
    }

    m_boundingSphereLocal.radius = std::sqrt(maxDistanceSquared);

    updateBoundingSphere();
    m_vertexBufferDirty = true;
}

void Mesh::Impl::verticesUvs(VectorView<glm::vec2> uvs)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), uvs.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].uv = uvs[i];
    }

    m_vertexBufferDirty = true;
}

void Mesh::Impl::verticesNormals(VectorView<glm::vec3> normals)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), normals.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].normal = glm::normalize(normals[i]);
    }

    m_vertexBufferDirty = true;
}

void Mesh::Impl::verticesTangents(VectorView<glm::vec4> tangents)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), tangents.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].tangent = tangents[i];
    }

    m_vertexBufferDirty = true;
}

void Mesh::Impl::indices(VectorView<uint16_t> indices, bool flipTriangles)
{
    setIndices(m_indices, indices, flipTriangles, m_vertices.size());
    createIndexBuffer();
}

void Mesh::Impl::indices(VectorView<uint8_t> indices, bool flipTriangles)
{
    setIndices(m_indices, indices, flipTriangles, m_vertices.size());
    createIndexBuffer();
}

// ----- Material

void Mesh::Impl::material(Material& material)
{
    m_material = &material;
}

// ----- Debug

void Mesh::Impl::boundingSphereVisible(bool boundingSphereVisible)
{
    if (m_boundingSphereVisible == boundingSphereVisible) return;
    m_boundingSphereVisible = boundingSphereVisible;

    if (m_boundingSphereVisible) {
        m_boundingSphereMesh = &m_baseScene.make<Mesh>();
        m_boundingSphereMesh->wireframed(true);

        buildSphereMesh(*m_boundingSphereMesh);

        updateBoundingSphere();
    }
    else {
        m_baseScene.remove(*m_boundingSphereMesh);
        m_boundingSphereMesh = nullptr;
    }
}

// ----- Internal

void Mesh::Impl::updateTransform()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_transform = glm::mat4(1.f);
    m_transform = glm::scale(m_transform, m_scaling);
    m_transform = glm::mat4(m_rotation) * m_transform;
    m_transform[3] = glm::vec4(m_translation, 1.f);
}

void Mesh::Impl::updateBoundingSphere()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // @note The world-space bounding sphere is less
    // precise than the local because it is based on the bounding box
    // and not the exact vertices.
    auto boundingBoxExtent = m_scaling * m_boundingBoxExtentLocal;
    auto radius = glm::length(boundingBoxExtent / 2.f);

    m_boundingSphere.center = glm::vec3(m_transform * glm::vec4(m_boundingSphereLocal.center, 1));
    m_boundingSphere.radius = radius;

    if (m_boundingSphereVisible) {
        m_boundingSphereMesh->translation(m_boundingSphere.center);
        m_boundingSphereMesh->scaling(m_boundingSphere.radius);
    }
}

void Mesh::Impl::updateBindings()
{
    auto transposeTransform = glm::transpose(m_transform);
    m_ubo.transform0 = transposeTransform[0];
    m_ubo.transform1 = transposeTransform[1];
    m_ubo.transform2 = transposeTransform[2];
}

void Mesh::Impl::createVertexBuffer()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    // Unlit (just holding vertices' positions)
    vk::DeviceSize bufferSize = sizeof(vulkan::UnlitVertex) * m_unlitVertices.size();
    m_unlitVertexBufferHolder.create(vk::BufferUsageFlagBits::eVertexBuffer, bufferSize);
    m_unlitVertexBufferHolder.copy(m_unlitVertices.data(), bufferSize);

    // Lit
    bufferSize = sizeof(vulkan::Vertex) * m_vertices.size();
    m_vertexBufferHolder.create(vk::BufferUsageFlagBits::eVertexBuffer, bufferSize);
    m_vertexBufferHolder.copy(m_vertices.data(), bufferSize);

    m_vertexBufferDirty = false;
}

void Mesh::Impl::createIndexBuffer()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    if (m_indices.empty()) {
        logger.warning("magma.vulkan.mesh") << "No indices provided. The mesh will not be visible." << std::endl;
        return;
    }

    vk::DeviceSize bufferSize = sizeof(uint16_t) * m_indices.size();

    m_indexBufferHolder.create(vk::BufferUsageFlagBits::eIndexBuffer, bufferSize);
    m_indexBufferHolder.copy(m_indices.data(), bufferSize);
}

// ----- Internal interface -----

void Mesh::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset,
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

    // Add the vertex buffer
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, &m_vertexBufferHolder.buffer(), offsets);
    commandBuffer.bindIndexBuffer(m_indexBufferHolder.buffer(), 0, vk::IndexType::eUint16);

    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                pushConstantOffset, sizeof(vulkan::MeshUbo), &m_ubo);

    // Draw
    commandBuffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);
}

void Mesh::Impl::renderUnlit(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset)
{
    if (m_unlitVertices.empty() || m_indices.empty()) return;

    // Add the vertex buffer
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, &m_unlitVertexBufferHolder.buffer(), offsets);
    commandBuffer.bindIndexBuffer(m_indexBufferHolder.buffer(), 0, vk::IndexType::eUint16);

    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                pushConstantOffset, sizeof(vulkan::MeshUbo), &m_ubo);

    // Draw
    commandBuffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);
}
