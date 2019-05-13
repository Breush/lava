#pragma once

#include <lava/sill/mesh-primitive.hpp>

#include <lava/sill/game-engine.hpp>
#include <lava/sill/material.hpp>

namespace lava::sill {
    class MeshPrimitive::Impl {
    public:
        Impl(GameEngine& engine);
        ~Impl();

        magma::Mesh* magma() { return m_magma; }
        const magma::Mesh* magma() const { return m_magma; }

        void computeFlatNormals() { m_magma->computeFlatNormals(); }
        void computeTangents() { m_magma->computeTangents(); }

        // Geometry
        void verticesCount(const uint32_t count) { m_magma->verticesCount(count); }
        void verticesPositions(VectorView<glm::vec3> positions) { m_magma->verticesPositions(positions); }
        void verticesUvs(VectorView<glm::vec2> uvs) { m_magma->verticesUvs(uvs); }
        void verticesNormals(VectorView<glm::vec3> normals) { m_magma->verticesNormals(normals); }
        void verticesTangents(VectorView<glm::vec4> tangents) { m_magma->verticesTangents(tangents); }
        void indices(VectorView<uint16_t> indices, bool flipTriangles) { m_magma->indices(indices, flipTriangles); }
        void indices(VectorView<uint8_t> indices, bool flipTriangles) { m_magma->indices(indices, flipTriangles); }

        // Material
        Material& material() { return *m_material; }
        void material(Material& material)
        {
            m_material = &material;
            m_magma->material(material.magma());
        }
        void translucent(bool translucent) { m_magma->translucent(translucent); }

    private:
        // References
        GameEngine::Impl& m_engine;
        Material* m_material = nullptr;

        // Resources
        magma::Mesh* m_magma = nullptr;
    };
}
