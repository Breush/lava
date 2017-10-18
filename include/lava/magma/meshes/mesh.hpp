#pragma once

#include <lava/magma/meshes/i-mesh.hpp>

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace lava::magma {
    class Material;
    class RenderScene;
}

namespace lava::magma {
    /**
     * A mesh, holding geometry and transform.
     */
    class Mesh final : public IMesh {
    public:
        Mesh(RenderScene& scene);
        ~Mesh();

        // IMesh
        virtual bool translucent() const override final;
        virtual void translucent(bool opaque) override final;
        IMesh::Impl& interfaceImpl() override final;

        const glm::mat4& transform() const;
        void transform(const glm::mat4& transform);
        void positionAdd(const glm::vec3& delta);
        void rotationAdd(const glm::vec3& axis, float angleDelta);

        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void verticesNormals(const std::vector<glm::vec3>& normals);
        void verticesTangents(const std::vector<glm::vec4>& tangents);
        void indices(const std::vector<uint16_t>& indices);

        Material& material();
        void material(Material& material);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
