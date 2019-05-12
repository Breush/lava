#pragma once

#include <lava/sill/mesh.hpp>

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
        std::unique_ptr<Mesh> mesh = nullptr;

        /// The local transform of the node, updating with animation.
        glm::mat4 localTransform = glm::mat4(1.f);

        /// The last known world transform of the node.
        glm::mat4 worldTransform = glm::mat4(1.f);

        /// A list of all children.
        std::vector<MeshNode*> children;

        /// The parent will be set automatically by MeshComponent if the children are specified.
        MeshNode* parent = nullptr;

    public:
        /// User-set local transform, with no animation.
        inline const glm::mat4& transform() const { return m_transform; }
        inline void transform(const glm::mat4& transform)
        {
            m_transform = transform;
            localTransform = transform;

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
        glm::quat m_rotation = glm::quat(0.f, 0.f, 0.f, 1.f);
        glm::vec3 m_scaling = glm::vec3(1.f);
    };
}
