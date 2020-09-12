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
        std::shared_ptr<MeshGroup> meshGroup = nullptr;

        /// The local transform of the node, updating with animation.
        glm::mat4 localTransform = glm::mat4(1.f);

        /// The last known entity-space transform of the node, updating with animation.
        /// Basically always parent->plainLocalTransform * localTransform.
        glm::mat4 plainLocalTransform = glm::mat4(1.f);

        /// A list of all children.
        /// These indices are relative position from ourselves.
        /// Which means that the corresponding node is (this + children[0]),
        /// because the data locality is ensured within the same mesh component.
        std::vector<int16_t> children;

        /// When communicating with the meshGroup, the instance index of the mesh.
        /// Automatically recomputed on node modifications by MeshComponent.
        uint32_t instanceIndex = 0u;

        /// The parent will be set automatically by MeshComponent if the children are specified.
        /// A parent set to self (i.e. 0) means that we have no parent.
        int16_t parent = 0;

    public:
        /// User-set local transform, with no animation.
        inline const glm::mat4& transform() const { return m_transform; }
        inline void transform(const glm::mat4& transform)
        {
            m_transform = transform;
            localTransform = transform;
            plainLocalTransform = transform;

            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(m_transform, m_scaling, m_rotation, m_translation, skew, perspective);
        }

        inline const glm::vec3& translation() const { return m_translation; }
        inline const glm::quat& rotation() const { return m_rotation; }
        inline const glm::vec3& scaling() const { return m_scaling; }

    private:
        glm::mat4 m_transform = glm::mat4(1.f);
        glm::vec3 m_translation = glm::vec3(0.f);
        glm::quat m_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        glm::vec3 m_scaling = glm::vec3(1.f);
    };
}
