#pragma once

#include <string>
#include <vector>

namespace lava::magma {
    class Mesh;
}

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    /**
     * Holds the primitives.
     */
    class MeshGroup {
    public:
        MeshGroup(GameEngine& engine);
        ~MeshGroup();

        // User info
        const std::string& name() const { return m_name; }
        void name(const std::string& name) { m_name = name; }

        // Primitives
        const magma::Mesh& primitive(uint32_t index) const { return *m_primitives[index]; }
        magma::Mesh& primitive(uint32_t index) { return *m_primitives[index]; }

        const std::vector<magma::Mesh*>& primitives() const { return m_primitives; }
        std::vector<magma::Mesh*>& primitives()  { return m_primitives; }

        magma::Mesh& addPrimitive();

        /// Changes the transform of all primitives.
        void transform(const glm::mat4& transform);

    private:
        GameEngine& m_engine;

        // Resources
        std::string m_name;
        std::vector<magma::Mesh*> m_primitives;
    };
}
