#pragma once

#include <lava/magma/meshes/i-mesh.hpp>

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace lava::magma {
    class IMaterial;
    class RenderScene;
}

namespace lava::magma {
    /**
     * A mesh, holding geometry and transform.
     */
    class Mesh final : public IMesh {
    public:
        Mesh(RenderScene& scene);
        Mesh(RenderScene& scene, const std::string& fileName);
        ~Mesh();

        // IMesh
        IMesh::Impl& interfaceImpl() override final;

        const glm::mat4& worldTransform() const;
        void positionAdd(const glm::vec3& delta);

        void load(const std::string& fileName);
        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void verticesNormals(const std::vector<glm::vec3>& normals);
        void verticesTangents(const std::vector<glm::vec4>& tangents);
        void indices(const std::vector<uint16_t>& indices);

        IMaterial& material();
        void material(IMaterial& material);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
        RenderScene& m_scene; // @todo We should not be aware of that here!
    };
}
