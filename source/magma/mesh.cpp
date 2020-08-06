#include <lava/magma/mesh.hpp>

#include <lava/chamber/mikktspace.hpp>
#include <lava/magma/scene.hpp>

#include "./mesh-tools.hpp"

// @todo Could be #ifdef according to backend
#include "./aft-vulkan/mesh-aft.hpp"

using namespace lava;
using namespace lava::chamber;
using namespace lava::magma;

namespace {
    /// Initialize the targetIndices array from the vector view, fliping triangles is asked.
    template <class UInt>
    void setIndices(std::vector<uint16_t>& targetIndices, const VectorView<UInt>& indices, bool flipTriangles,
                    uint32_t verticesCount);
}

Mesh::Mesh(Scene& scene)
    : m_scene(scene)
{
    new (&aft()) MeshAft(*this, m_scene);

    updateUbo();
}

Mesh::~Mesh()
{
    if (m_debugBoundingSphere) {
        debugBoundingSphere(false);
    }

    aft().~MeshAft();
}

// ----- Transform

void Mesh::transform(const glm::mat4& transform)
{
    m_transform = transform;

    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(m_transform, m_scaling, m_rotation, m_translation, skew, perspective);

    updateBoundingSphere();
    updateUbo();
}

void Mesh::translation(const glm::vec3& translation)
{
    m_translation = translation;
    updateTransform();
}

void Mesh::rotation(const glm::quat& rotation)
{
    m_rotation = rotation;
    updateTransform();
}

void Mesh::scaling(const glm::vec3& scaling)
{
    m_scaling = scaling;
    updateTransform();
}

// ----- Geometry

void Mesh::verticesCount(const uint32_t count)
{
    m_unlitVertices.resize(count);
    m_vertices.resize(count);
}

void Mesh::verticesPositions(const VectorView<glm::vec3>& positions)
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
    m_boundingSphereGeometry.center = (minRange + maxRange) / 2.f;
    m_boundingBoxExtentGeometry = maxRange - minRange;

    auto maxDistanceSquared = 0.f;
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), positions.size());
    for (uint32_t i = 0u; i < length; ++i) {
        // Storing the position.
        const auto& position = positions[i];
        m_vertices[i].pos = position;
        m_unlitVertices[i].pos = position;

        // Finding the bounding sphere radius.
        auto vertexVector = position - m_boundingSphereGeometry.center;
        maxDistanceSquared = glm::dot(vertexVector, vertexVector);
    }

    m_boundingSphereGeometry.radius = std::sqrt(maxDistanceSquared);

    updateBoundingSphere();
    aft().foreVerticesChanged();
}

void Mesh::verticesUvs(const VectorView<glm::vec2>& uvs)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), uvs.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].uv = uvs[i];
    }

    aft().foreVerticesChanged();
}

void Mesh::verticesNormals(const VectorView<glm::vec3>& normals)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), normals.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].normal = glm::normalize(normals[i]);
    }

    aft().foreVerticesChanged();
}

void Mesh::verticesTangents(const VectorView<glm::vec4>& tangents)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), tangents.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].tangent = tangents[i];
    }

    aft().foreVerticesChanged();
}

void Mesh::indices(const VectorView<uint32_t>& indices, bool flipTriangles)
{
    setIndices(m_indices, indices, flipTriangles, m_vertices.size());
    aft().foreIndicesChanged();
}

void Mesh::indices(const VectorView<uint16_t>& indices, bool flipTriangles)
{
    setIndices(m_indices, indices, flipTriangles, m_vertices.size());
    aft().foreIndicesChanged();
}

void Mesh::indices(const VectorView<uint8_t>& indices, bool flipTriangles)
{
    setIndices(m_indices, indices, flipTriangles, m_vertices.size());
    aft().foreIndicesChanged();
}

void Mesh::computeFlatNormals()
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

    aft().foreVerticesChanged();
}

void Mesh::computeTangents()
{
    SMikkTSpaceInterface tsInterface;
    tsInterface.m_getNumFaces = [](const SMikkTSpaceContext* pContext) -> int {
        const auto& self = *reinterpret_cast<const Mesh*>(pContext->m_pUserData);
        return self.m_indices.size() / 3;
    };
    tsInterface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* /* pContext */, const int /* iFace */) -> int { return 3; };
    tsInterface.m_getPosition = [](const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert) {
        const auto& self = *reinterpret_cast<const Mesh*>(pContext->m_pUserData);
        auto vertexIndex = self.m_indices[3 * iFace + iVert];
        fvPosOut[0] = self.m_vertices[vertexIndex].pos[0];
        fvPosOut[1] = self.m_vertices[vertexIndex].pos[1];
        fvPosOut[2] = self.m_vertices[vertexIndex].pos[2];
    };
    tsInterface.m_getNormal = [](const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert) {
        const auto& self = *reinterpret_cast<const Mesh*>(pContext->m_pUserData);
        auto vertexIndex = self.m_indices[3 * iFace + iVert];
        fvNormOut[0] = self.m_vertices[vertexIndex].normal[0];
        fvNormOut[1] = self.m_vertices[vertexIndex].normal[1];
        fvNormOut[2] = self.m_vertices[vertexIndex].normal[2];
    };
    tsInterface.m_getTexCoord = [](const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert) {
        const auto& self = *reinterpret_cast<const Mesh*>(pContext->m_pUserData);
        auto vertexIndex = self.m_indices[3 * iFace + iVert];
        fvTexcOut[0] = self.m_vertices[vertexIndex].uv[0];
        fvTexcOut[1] = self.m_vertices[vertexIndex].uv[1];
    };

    m_temporaryVertices.clear();
    m_temporaryVertices.reserve(m_vertices.size());
    tsInterface.m_setTSpaceBasic = [](const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign,
                                      const int iFace, const int iVert) {
        auto& self = *reinterpret_cast<Mesh*>(pContext->m_pUserData);
        auto vertexIndex = self.m_indices[3 * iFace + iVert];

        Vertex v;
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
    m_vertices = std::move(m_temporaryVertices);
    auto verticeCount = m_vertices.size();
    m_indices.resize(verticeCount);
    m_unlitVertices.resize(verticeCount);
    for (auto i = 0u; i < verticeCount; ++i) {
        m_indices[i] = i;
        m_unlitVertices[i].pos = m_vertices[i].pos;
    }

    aft().foreVerticesChanged();
    aft().foreIndicesChanged();

    m_temporaryVertices.resize(0);
}

// ----- Material

void Mesh::material(MaterialPtr material)
{
    m_material = std::move(material);
}

// ----- Debug

void Mesh::debugBoundingSphere(bool debugBoundingSphere)
{
    if (m_debugBoundingSphere == debugBoundingSphere) return;
    m_debugBoundingSphere = debugBoundingSphere;

    if (m_debugBoundingSphere) {
        m_debugBoundingSphereMesh = &m_scene.make<Mesh>();
        m_debugBoundingSphereMesh->category(RenderCategory::Wireframe);

        buildSphereMesh(*m_debugBoundingSphereMesh);

        updateBoundingSphere();
    }
    else {
        m_scene.remove(*m_debugBoundingSphereMesh);
        m_debugBoundingSphereMesh = nullptr;
    }
}

// ----- Updates

void Mesh::updateUbo()
{
    auto transposeTransform = glm::transpose(m_transform);
    m_ubo.transform0 = transposeTransform[0];
    m_ubo.transform1 = transposeTransform[1];
    m_ubo.transform2 = transposeTransform[2];
}

void Mesh::updateTransform()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_transform = glm::scale(glm::mat4(1.f), m_scaling);
    m_transform = glm::mat4(m_rotation) * m_transform;
    m_transform[3] = glm::vec4(m_translation, 1.f);

    updateBoundingSphere();
    updateUbo();
}

void Mesh::updateBoundingSphere()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // @note The world-space bounding sphere is less
    // precise than the geometry-space one because it is based on the bounding box
    // and not the exact vertices.
    auto boundingBoxExtent = m_scaling * m_boundingBoxExtentGeometry;
    auto radius = glm::length(boundingBoxExtent / 2.f);

    m_boundingSphere.center = glm::vec3(m_transform * glm::vec4(m_boundingSphereGeometry.center, 1));
    m_boundingSphere.radius = radius;

    if (m_debugBoundingSphere) {
        m_debugBoundingSphereMesh->translation(m_boundingSphere.center);
        m_debugBoundingSphereMesh->scaling(m_boundingSphere.radius);
    }
}

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
            logger.error("magma.vulkan.mesh") << "Some indices are refering indices bigger than the vertices count." << std::endl;
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
