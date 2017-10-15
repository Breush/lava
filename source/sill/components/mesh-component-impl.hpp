#pragma once

#include <lava/sill/components/mesh-component.hpp>

#include <lava/magma/meshes/mesh.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class TransformComponent;
}

namespace lava::sill {
    class MeshComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);
        ~Impl();

        // IComponent
        void update() override final;
        void postUpdate() override final {}

        // MeshComponent
        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void verticesNormals(const std::vector<glm::vec3>& normals);
        void verticesTangents(const std::vector<glm::vec4>& tangents);
        void indices(const std::vector<uint16_t>& indices);

        void material(magma::IMaterial& material);

    private:
        // References
        TransformComponent& m_transformComponent;

        // Resources
        magma::Mesh* m_mesh = nullptr;
    };
}
