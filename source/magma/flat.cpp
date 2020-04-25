#include <lava/magma/flat.hpp>

#include <lava/magma/scene.hpp>

// @todo Could be #ifdef according to backend
#include "./aft-vulkan/flat-aft.hpp"

using namespace lava;
using namespace lava::chamber;
using namespace lava::magma;

namespace {
    /// Initialize the targetIndices array from the vector view.
    template <class UInt>
    void setIndices(std::vector<uint16_t>& targetIndices, const VectorView<UInt>& indices, uint32_t verticesCount);
}

Flat::Flat(Scene& scene)
    : m_scene(scene)
{
    new (&aft()) FlatAft(*this, m_scene);

    updateUbo();
}

Flat::~Flat()
{
    aft().~FlatAft();
}

// ----- Transform

void Flat::transform(const glm::mat3& transform)
{
    m_transform = transform;

    m_scaling.x = std::sqrt(transform[0].x * transform[0].x + transform[0].y * transform[0].y);
    m_scaling.x = std::sqrt(transform[1].x * transform[1].x + transform[1].y * transform[1].y);

    m_rotation = atan2(transform[0].y, transform[0].x);

    m_translation.x = transform[2].x;
    m_translation.y = transform[2].y;

    updateUbo();
}

void Flat::translation(const glm::vec2& translation)
{
    m_translation = translation;
    updateTransform();
}

void Flat::rotation(float rotation)
{
    m_rotation = rotation;
    updateTransform();
}

void Flat::scaling(const glm::vec2& scaling)
{
    m_scaling = scaling;
    updateTransform();
}

// ----- Geometry

void Flat::verticesCount(const uint32_t count)
{
    m_vertices.resize(count);
}

void Flat::verticesPositions(VectorView<glm::vec2> positions)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), positions.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].pos = positions[i];
    }

    aft().foreVerticesChanged();
}

void Flat::verticesUvs(VectorView<glm::vec2> uvs)
{
    auto length = std::min(static_cast<uint32_t>(m_vertices.size()), uvs.size());
    for (uint32_t i = 0u; i < length; ++i) {
        m_vertices[i].uv = uvs[i];
    }

    aft().foreVerticesChanged();
}

void Flat::indices(VectorView<uint16_t> indices)
{
    setIndices(m_indices, indices, m_vertices.size());
    aft().foreIndicesChanged();
}

void Flat::indices(VectorView<uint8_t> indices)
{
    setIndices(m_indices, indices, m_vertices.size());
    aft().foreIndicesChanged();
}

// ----- Material

void Flat::material(MaterialPtr material)
{
    m_material = std::move(material);
}

// ----- Updates

void Flat::updateUbo()
{
    m_ubo.transform = glm::vec4{m_transform[0].x, m_transform[0].y, m_transform[1].x, m_transform[1].y};
    m_ubo.translation = glm::vec4{m_translation, 0, 0};
}

void Flat::updateTransform()
{
    m_transform = glm::scale(glm::mat3(1.f), m_scaling);
    m_transform = glm::rotate(glm::mat3(1.f), m_rotation) * m_transform;
    m_transform[2] = glm::vec3(m_translation, 1.f);

    updateUbo();
}

namespace {
    template <class UInt>
    inline void setIndices(std::vector<uint16_t>& targetIndices, const VectorView<UInt>& indices, uint32_t verticesCount)
    {
        auto length = indices.size();
        targetIndices.resize(length);

        // Fast coherency check
        if (indices[0] >= verticesCount || indices[length / 2] >= verticesCount) {
            logger.warning("magma.vulkan.flat") << "Vertices count: " << verticesCount << std::endl;
            logger.warning("magma.vulkan.flat")
                << "Wrong vertex index: " << static_cast<uint16_t>(std::max(indices[0], indices[length / 2])) << std::endl;
            logger.error("magma.vulkan.flat") << "Some indices are refering indices bigger than the vertices count." << std::endl;
        }

        for (auto i = 0u; i < length; ++i) {
            targetIndices[i] = indices[i];
        }
    }
}
