#pragma once

#include <lava/sill/mesh-primitive.hpp>

#include <string>
#include <vector>

namespace lava::sill {
    /**
     * Holds the primitives.
     *
     * As it should always be attached to a MeshNode,
     * there is no way to set the transform here.
     */
    class Mesh {
    public:
        Mesh();
        ~Mesh();

        // User info
        const std::string& name() const;
        void name(const std::string& name);

        /// Primitives
        MeshPrimitive& primitive(uint32_t index);
        const std::vector<MeshPrimitive>& primitives() const;
        std::vector<MeshPrimitive>& primitives();
        void primitives(std::vector<MeshPrimitive>&& primitives);
        MeshPrimitive& addPrimitive(GameEngine& engine);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
