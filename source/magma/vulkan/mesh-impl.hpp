#pragma once

#include <lava/magma/mesh.hpp>

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../bounding-sphere.hpp"
#include "./holders/buffer-holder.hpp"
#include "./ubos.hpp"
#include "./vertex.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of a mesh.
     */
    class Mesh::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl();

        // Internal interface
        void init();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset,
                    uint32_t materialDescriptorSetIndex);
        void renderUnlit(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset);

        const BoundingSphere& boundingSphere() const { return m_boundingSphere; }

        // Mesh
        const glm::mat4& transform() const { return m_transform; }
        void transform(const glm::mat4& transform);
        const glm::vec3& translation() const { return m_translation; }
        void translation(const glm::vec3& translation);
        void translate(const glm::vec3& delta) { translation(m_translation + delta); }
        const glm::quat& rotation() const { return m_rotation; }
        void rotation(const glm::quat& rotation);
        void rotate(const glm::vec3& axis, float angleDelta);
        const glm::vec3& scaling() const { return m_scaling; }
        void scaling(const glm::vec3& scaling);
        void scaling(float factor) { scaling({factor, factor, factor}); }
        void scale(const glm::vec3& factor) { scaling(m_scaling * factor); }
        void scale(float factor) { scaling(m_scaling * factor); }

        void verticesCount(const uint32_t count);
        void verticesPositions(VectorView<glm::vec3> positions);
        void verticesUvs(VectorView<glm::vec2> uvs);
        void verticesNormals(VectorView<glm::vec3> normals);
        void verticesTangents(VectorView<glm::vec4> tangents);
        void indices(VectorView<uint16_t> indices, bool flipTriangles);
        void indices(VectorView<uint8_t> indices, bool flipTriangles);

        bool canCastShadows() const { return m_canCastShadows && !m_wireframed; }
        void canCastShadows(bool canCastShadows) { m_canCastShadows = canCastShadows; }

        bool vrRenderable() const { return m_vrRenderable; }
        void vrRenderable(bool vrRenderable) { m_vrRenderable = vrRenderable; }

        void computeFlatNormals();
        void computeTangents();

        Material& material() { return *m_material; }
        void material(Material& material);

        bool translucent() const { return m_translucent; }
        void translucent(bool translucent) { m_translucent = translucent; }

        bool wireframed() const { return m_wireframed; }
        void wireframed(bool wireframed) { m_wireframed = wireframed; }

        bool boundingSphereVisible() const { return m_boundingSphereVisible; }
        void boundingSphereVisible(bool boundingSphereVisible);

    private:
        void updateTransform();
        void updateBoundingSphere();
        void updateBindings();
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        // References
        RenderScene& m_baseScene;
        RenderScene::Impl& m_scene;
        bool m_initialized = false;

        // Data
        std::vector<vulkan::UnlitVertex> m_unlitVertices;
        std::vector<vulkan::Vertex> m_vertices;
        std::vector<uint16_t> m_indices;
        Material* m_material = nullptr;
        bool m_canCastShadows = true;
        bool m_vrRenderable = true;
        bool m_translucent = false;
        bool m_wireframed = false;
        bool m_boundingSphereVisible = false;

        // Computed
        BoundingSphere m_boundingSphereLocal;
        BoundingSphere m_boundingSphere;
        // Local bounding box dimenstion which is centered at m_boundingSphereLocal.center.
        glm::vec3 m_boundingBoxExtentLocal;

        // Debug
        Mesh* m_boundingSphereMesh = nullptr;

        // Node
        glm::mat4 m_transform = glm::mat4(1.f);
        glm::vec3 m_translation = glm::vec3(0.f);
        glm::quat m_rotation = glm::quat(0.f, 0.f, 0.f, 1.f);
        glm::vec3 m_scaling = glm::vec3(1.f);

        // Descriptor
        vulkan::MeshUbo m_ubo;

        // Vertices
        vulkan::BufferHolder m_unlitVertexBufferHolder;
        vulkan::BufferHolder m_vertexBufferHolder;
        vulkan::BufferHolder m_indexBufferHolder;
    };
}
