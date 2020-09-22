#pragma once

namespace lava::sill {
    class Entity;
}

namespace lava::sill {
    /// Interface for frame components.
    class IFrameComponent {
    public:
        virtual ~IFrameComponent() = default;

        /// Build up an entity based on this frame.
        virtual void makeEntity(Entity& entity) = 0;

        /// Be warn when a framed entity is removed.
        virtual void warnEntityRemoved(Entity& entity) = 0;

        // To be implemented
        // static std::string hrid();
    };
}
