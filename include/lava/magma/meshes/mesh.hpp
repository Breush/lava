#pragma once

#include <lava/magma/interfaces/mesh.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>

namespace lava::magma {
    class RmMaterial;
}

namespace lava::magma {
    /**
     * A mesh, holding geometry and transform.
     */
    class Mesh final : public IMesh {
    public:
        Mesh(RenderEngine& engine);
        Mesh(RenderEngine& engine, const std::string& fileName);
        ~Mesh();

        // IMesh
        const glm::mat4& worldTransform() const override final;
        UserData render(UserData data) override final;

        void positionAdd(const glm::vec3& delta);

        void load(const std::string& fileName);
        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void verticesNormals(const std::vector<glm::vec3>& normals);
        void verticesTangents(const std::vector<glm::vec4>& tangents);
        void indices(const std::vector<uint16_t>& indices);

        // @todo Why isn't that a IMaterial?
        RmMaterial& material();
        void material(RmMaterial& material);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
        RenderEngine& m_engine;
    };
}
