#pragma once

#include <lava/sill/mesh.hpp>

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
        std::unique_ptr<Mesh> mesh = nullptr;

        /// The local transform of the node.
        glm::mat4 transform;

        /// A list of all children.
        std::vector<MeshNode*> children;

        /// The parent will be set automatically by MeshComponent if the children are specified.
        MeshNode* parent = nullptr;
    };
}
