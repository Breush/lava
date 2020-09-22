#pragma once

namespace lava::sill {
    class Entity;
}

namespace lava::sill {
    /// Interface for components.
    class IComponent {
    public:
        IComponent(Entity& entity)
            : m_entity(entity)
        {
        }
        virtual ~IComponent() = default;

        Entity& entity() { return m_entity; }
        const Entity& entity() const { return m_entity; }

        /// Called regularly, dt is expressed in seconds and constant between frames.
        virtual void update(float /* dt */) {}

        /// Called on a frame basis.
        virtual void updateFrame() {}

        // To be implemented
        // static std::string hrid();

    protected:
        Entity& m_entity;
    };
}
