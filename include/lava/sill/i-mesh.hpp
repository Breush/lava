#pragma once

#include <lava/core/bounding-sphere.hpp>
#include <lava/core/render-category.hpp>
#include <lava/sill/mesh-animation.hpp>
#include <lava/sill/mesh-group.hpp>
#include <lava/sill/mesh-node.hpp>

namespace lava::magma {
    class Scene;
}

namespace lava::sill {
    // Provides mesh construction API.
    class IMesh {
    public:
        IMesh(magma::Scene& scene, bool autoInstancingEnabled);

        magma::Scene& scene() { return m_scene; }
        const magma::Scene& scene() const { return m_scene; }

        /**
         * @name Nodes
         *
         * @note We purposefully do not give write access to nodes
        // so that one cannot create a MeshGroup manually.
         */
        /// @{
        //
        const std::vector<MeshNode>& nodes() const { return m_nodes; }

        void reserveNodes(uint32_t nodesCount);
        uint32_t addNode();
        uint32_t addInstancedNode(uint32_t sourceNodeIndex);
        // @fixme When removing instanced nodes, we should warn the primitives
        // that there is one less instance.
        void removeNodes() { m_nodes.clear(); }

        MeshGroup& nodeMakeGroup(uint32_t nodeIndex);
        void nodeName(uint32_t nodeIndex, std::string name);
        const glm::mat4& nodeMatrix(uint32_t nodeIndex) const { return m_nodes[nodeIndex].matrix; }
        void nodeMatrix(uint32_t nodeIndex, const glm::mat4& matrix);
        void nodeAddAbsoluteChild(uint32_t nodeIndex, uint32_t childNodeIndex);
        /// @}

        /**
         * @name Animations
         */
        /// @{
        void addAnimation(const std::string& hrid, const MeshAnimation& animation);
        /// @}

        /**
         * @name Attributes
         */
        /// @{
        /// Path of the file if read from any.
        const std::string& path() const { return m_path; }
        void path(const std::string& path) { m_path = path; }

        /// Changes the render category of all primitives.
        void renderCategory(RenderCategory renderCategory);

        /// Changes the enableness of all primitives.
        void enabled(bool enabled);

        /// Bounding sphere around all bounding spheres of primitives.
        BoundingSphere boundingSphere() const;

        void printHierarchy(std::ostream& s) const;
        /// @}

    protected:
        void updateNodesEntitySpaceMatrices();

    protected:
        magma::Scene& m_scene;
        bool m_autoInstancingEnabled = true;

        // Nodes
        std::vector<MeshNode> m_nodes;
        bool m_nodesDirty = false;

        // Animations
        std::unordered_map<std::string, MeshAnimation> m_animations;

        // Attributes
        std::string m_path;
    };
}
