#pragma once

#include <lava/sill/mesh-group.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <string>
#include <vector>

namespace lava::sill {
    /**
     * A node for meshes hierarchy.
     *
     * Definition is very close to the glTF2.0 concept of node.
     */
    struct MeshNode {
        /// Mainly for debugging proposes.
        std::string name;

        /// The optional mesh geometry (and material).
        std::shared_ptr<MeshGroup> group = nullptr;

        /// The user-set local (parent-space) matrix of the node and its decompose.
        /// Never updated with animation.
        glm::mat4 matrix = glm::mat4(1.f);
        glm::vec3 translation = glm::vec3(0.f);
        glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        glm::vec3 scaling = glm::vec3(1.f);

        /// The local (parent-space) matrix of the node.
        /// Updated with animation.
        glm::mat4 parentSpaceMatrix = glm::mat4(1.f);

        /// The last known entity-space matrix of the node.
        /// Basically always parent.entitySpaceMatrix * matrix.
        /// Updated with animation.
        glm::mat4 entitySpaceMatrix = glm::mat4(1.f);

        /// A list of all children.
        /// These indices are relative position from ourselves.
        /// Which means that the corresponding node is (this + children[0]),
        /// because the data locality is ensured within the same mesh component.
        std::vector<int16_t> children;

        /// The parent will be set automatically by IMesh if the children are specified.
        /// A parent set to self (i.e. 0) means that we have no parent.
        int16_t parent = 0;

        /// When communicating with the group, the instance index of the mesh.
        /// Automatically recomputed on node modifications by IMesh.
        uint32_t instanceIndex = 0u;
    };
}
