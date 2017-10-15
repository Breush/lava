#pragma once

namespace lava::sill {
    /// Interface for components.
    class IComponent {
    public:
        virtual ~IComponent() = default;

        /// Called once per frame, respecting components dependencies.
        virtual void update() = 0;

        /// Called after all update of components.
        virtual void postUpdate() = 0;

        // To be implemented
        // static std::string hrid();
    };
}
