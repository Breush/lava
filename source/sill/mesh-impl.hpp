#pragma once

#include <lava/sill/mesh.hpp>

#include <lava/sill/game-entity.hpp>

namespace lava::sill {
    class Mesh::Impl {
    public:
        // Mesh user info
        const std::string& name() const { return m_name; }
        void name(const std::string& name) { m_name = name; }

        // Mesh primitives
        MeshPrimitive& primitive(uint32_t index) { return m_primitives[index]; }
        const std::vector<MeshPrimitive>& primitives() const { return m_primitives; }
        std::vector<MeshPrimitive>& primitives() { return m_primitives; }
        void primitives(std::vector<MeshPrimitive>&& primitives);
        MeshPrimitive& addPrimitive(GameEngine& engine);

        // Internal interface
        void transform(const glm::mat4& transform);

    private:
        // Resources
        std::string m_name;
        std::vector<MeshPrimitive> m_primitives;
    };
}
