#pragma once

#include <lava/magma/mesh.hpp>

#include <string>
#include <vector>

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    /**
     * Holds the primitives.
     *
     * As it should always be attached to a MeshNode,
     * there is no way to set the transform here.
     *
     * @fixme Could be renamed MeshGroup for consistency.
     */
    class Mesh {
    public:
        Mesh(GameEngine& engine);
        ~Mesh();

        // User info
        const std::string& name() const;
        void name(const std::string& name);

        /// Primitives
        const magma::Mesh& primitive(uint32_t index) const;
        magma::Mesh& primitive(uint32_t index);

        const std::vector<magma::Mesh*>& primitives() const;
        std::vector<magma::Mesh*>& primitives();

        magma::Mesh& addPrimitive();

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
