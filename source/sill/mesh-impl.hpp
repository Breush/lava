#pragma once

#include <lava/sill/mesh.hpp>

#include <lava/magma/mesh.hpp>
#include <lava/sill/game-entity.hpp>

namespace lava::sill {
    class Mesh::Impl {
    public:
        Impl(GameEngine& engine);
        ~Impl();

        // Mesh user info
        const std::string& name() const { return m_name; }
        void name(const std::string& name) { m_name = name; }

        // Mesh primitives
        const magma::Mesh& primitive(uint32_t index) const { return *m_primitives[index]; }
        magma::Mesh& primitive(uint32_t index) { return *m_primitives[index]; }

        const std::vector<magma::Mesh*>& primitives() const { return m_primitives; }
        std::vector<magma::Mesh*>& primitives() { return m_primitives; }

        magma::Mesh& addPrimitive();

        // Internal interface
        void transform(const glm::mat4& transform);

    private:
        GameEngine& m_engine;

        // Resources
        std::string m_name;
        std::vector<magma::Mesh*> m_primitives;
    };
}
