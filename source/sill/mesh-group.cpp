#include <lava/sill/mesh-group.hpp>

#include <lava/sill/game-engine.hpp>

using namespace lava;
using namespace lava::sill;

MeshGroup::MeshGroup(magma::Scene& scene)
    : MeshGroup(scene, true)
{
}

MeshGroup::MeshGroup(magma::Scene& scene, bool autoInstancingEnabled)
    : m_scene(scene)
    , m_autoInstancingEnabled(autoInstancingEnabled)
{
}

MeshGroup::~MeshGroup()
{
    for (auto& primitive : m_primitives) {
        m_scene.remove(*primitive);
    }
}

//----- Primitives

magma::Mesh& MeshGroup::addPrimitive()
{
    auto& mesh = m_scene.make<magma::Mesh>(m_autoInstancingEnabled ? 1u : 0u);
    m_primitives.emplace_back(&mesh);
    return mesh;
}

//----- Transforms

void MeshGroup::transform(const glm::mat4& transform, uint32_t instanceIndex)
{
    // @note Each magma::Mesh has its own transform,
    // there are no notions of primitive in magma.
    for (auto& primitive : m_primitives) {
        primitive->transform(transform, instanceIndex);
    }
}
