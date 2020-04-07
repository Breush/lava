#include <lava/sill/mesh-group.hpp>

#include <lava/sill/game-engine.hpp>

using namespace lava;
using namespace lava::sill;

MeshGroup::MeshGroup(GameEngine& engine)
    : m_engine(engine)
{
}

MeshGroup::~MeshGroup()
{
    for (auto& primitive : m_primitives) {
        // @fixme :AutoMaterialDelete
        if (primitive->material() != nullptr) {
            m_engine.scene().remove(*primitive->material());
        }
        m_engine.scene().remove(*primitive);
    }
}

//----- Primitives

magma::Mesh& MeshGroup::addPrimitive()
{
    auto& mesh = m_engine.scene().make<magma::Mesh>();
    m_primitives.emplace_back(&mesh);
    return mesh;
}

//----- Transforms

void MeshGroup::transform(const glm::mat4& transform)
{
    // @note Each magma::Mesh has its own transform,
    // there are no notions of primitive in magma.
    for (auto& primitive : m_primitives) {
        primitive->transform(transform);
    }
}
