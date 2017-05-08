#pragma once

#include <glm/vec2.hpp>
#include <vector>

namespace lava {
    class Engine;
}

namespace lava {
    /**
     * A mesh, holding geometry and transform.
     */
    class Mesh {
    public:
        Mesh(Engine& engine);
        ~Mesh();

        class Impl;
        Impl& impl() { return *m_impl; }

        void vertices(const std::vector<glm::vec2>& vertices);
        void indices(const std::vector<uint16_t>& indices);

    private:
        Impl* m_impl = nullptr;
    };
}
