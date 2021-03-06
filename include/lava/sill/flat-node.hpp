#pragma once

#include <lava/sill/flat-group.hpp>

namespace lava::sill {
    /**
     * A node for flats.
     */
    struct FlatNode {
        /// Mainly for debugging proposes.
        std::string name;

        /// The optional flat geometry (and material).
        std::unique_ptr<FlatGroup> flatGroup = nullptr;

        /// The local transform of the node.
        glm::mat3 localTransform = glm::mat3(1.f);

        bool localTransformChanged = true;

    public:
        /// User-set local transform, with no animation.
        inline void transform(const glm::mat3& transform)
        {
            localTransform = transform;
            localTransformChanged = true;
        }
    };
}
