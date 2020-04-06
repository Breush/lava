#pragma once

#include <lava/core/macros/aft.hpp>
#include <lava/core/vector-view.hpp>
#include <lava/magma/ubos.hpp>
#include <lava/magma/vertex.hpp>

namespace lava::magma {
    class FlatAft;
    class Material;
    class Scene;
}

namespace lava::magma {
    /**
     * Like a mesh but for flat renderers.
     */
    class Flat {
    public:
        Flat(Scene& scene);
        ~Flat();

        $aft_class(Flat);

        Scene& scene() { return m_scene; }
        const Scene& scene() const { return m_scene; }

        /**
         * @name Transform
         */
        /// @{
        /// The transform is `translation * rotation * scaling`.
        const glm::mat3& transform() const { return m_transform; }
        void transform(const glm::mat3& transform);

        const glm::vec2& translation() const { return m_translation; }
        void translation(const glm::vec2& translation);
        void translate(const glm::vec2& delta) { translation(m_translation + delta); }

        float rotation() const { return m_rotation; }
        void rotation(float rotation);
        void rotate(float angleDelta) { rotation(m_rotation + angleDelta); }

        const glm::vec2& scaling() const { return m_scaling; }
        void scaling(const glm::vec2& scaling);
        void scale(const glm::vec2& factors) { scaling(m_scaling * factors); }
        void scale(float factor) { scaling(m_scaling * factor); }
        /// @}

        /**
         * @name Geometry
         */
        /// @{
        uint32_t verticesCount() const { return m_vertices.size(); }
        void verticesCount(const uint32_t count);
        void verticesPositions(VectorView<glm::vec2> positions);
        void verticesUvs(VectorView<glm::vec2> uvs);
        void indices(VectorView<uint16_t> indices);
        void indices(VectorView<uint8_t> indices);

        const std::vector<FlatVertex>& vertices() const { return m_vertices; };
        const std::vector<uint16_t>& indices() const { return m_indices; };

        /// From current vertices positions, compute flat normals.
        void computeFlatNormals();

        /// From current vertices positions and normals, compute tangents.
        void computeTangents();
        /// @}

        /**
         * @name Material
         */
        /// @{
        /// The mesh's material. Can be nullptr.
        Material* material() { return m_material; }
        const Material* material() const { return m_material; }
        void material(Material& material);
        /// @}

        /**
         * @name Shader data
         */
        /// @{
        const FlatUbo& ubo() const { return m_ubo; }
        /// @}

    private:
        void updateUbo();
        void updateTransform();

    private:
        // ----- References
        Scene& m_scene;

        // ----- Transform
        glm::mat3 m_transform = glm::mat3(1.f);
        float m_rotation = 0.f;
        glm::vec2 m_translation = glm::vec2(0.f);
        glm::vec2 m_scaling = glm::vec2(1.f);

        // ----- Geometry
        std::vector<FlatVertex> m_vertices;
        std::vector<uint16_t> m_indices;

        // ----- Material
        Material* m_material = nullptr;

        // ----- Shader data
        FlatUbo m_ubo;
    };
}
