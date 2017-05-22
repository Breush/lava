#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

namespace lava {
    class RenderEngine;
}

namespace lava {
    /**
     * A mesh, holding geometry and transform.
     */
    class Mesh {
    public:
        Mesh(RenderEngine& engine);
        ~Mesh();

        class Impl;
        Impl& impl() { return *m_impl; }

        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesColors(const std::vector<glm::vec3>& colors);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void indices(const std::vector<uint16_t>& indices);

    private:
        Impl* m_impl = nullptr;
    };
}
