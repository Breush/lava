#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace lava::sill {
    class Material;
}

namespace lava::sill {
    class MeshComponent final : public IComponent {
    public:
        MeshComponent(GameEntity& entity);
        ~MeshComponent();

        // IComponent
        static std::string hrid() { return "mesh"; }
        void update() override final;

        /**
        * @name Geometry
        */
        /// @{
        void verticesCount(const uint32_t count);
        void verticesPositions(const std::vector<glm::vec3>& positions);
        void verticesUvs(const std::vector<glm::vec2>& uvs);
        void verticesNormals(const std::vector<glm::vec3>& normals);
        void verticesTangents(const std::vector<glm::vec4>& tangents);
        void indices(const std::vector<uint16_t>& indices);
        /// @}

        /**
        * @name Materials
        */
        /// @{
        bool translucent() const;
        void translucent(bool translucent);
        void material(Material& material);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
