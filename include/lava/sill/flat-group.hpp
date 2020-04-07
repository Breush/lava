#pragma once

#include <string>
#include <vector>

namespace lava::magma {
    class Flat;
}

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    /**
     * Holds the primitives.
     */
    class FlatGroup {
    public:
        FlatGroup(GameEngine& engine);
        ~FlatGroup();

        // User info
        const std::string& name() const { return m_name; }
        void name(const std::string& name) { m_name = name; }

        // Primitives
        const magma::Flat& primitive(uint32_t index) const { return *m_primitives[index]; }
        magma::Flat& primitive(uint32_t index) { return *m_primitives[index]; }

        const std::vector<magma::Flat*>& primitives() const { return m_primitives; }
        std::vector<magma::Flat*>& primitives() { return m_primitives; }

        magma::Flat& addPrimitive();

        /// Changes the transform of all primitives.
        void transform(const glm::mat4& transform);

    public:
        GameEngine& m_engine;

        // Resources
        std::string m_name;
        std::vector<magma::Flat*> m_primitives;
    };
}
