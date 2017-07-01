#pragma once

#include <lava/magma/interfaces/mesh.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>

namespace lava::magma {
    class MrrMaterial;
}

namespace lava::magma {
    /**
     * A mesh, holding geometry and transform.
     */
    class Mesh : public IMesh {
    public:
        Mesh(RenderEngine& engine);
        Mesh(RenderEngine& engine, const std::string& fileName);
        ~Mesh();

        void* render(void* data) override final;

        class Impl;
        Impl& impl() { return *m_impl; }

        void load(const std::string& fileName);
        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesNormals(const std::vector<glm::vec3>& normals);
        void verticesColors(const std::vector<glm::vec3>& colors);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void indices(const std::vector<uint16_t>& indices);
        void material(const MrrMaterial& material);

    private:
        Impl* m_impl = nullptr;
        RenderEngine& m_engine;
    };
}
