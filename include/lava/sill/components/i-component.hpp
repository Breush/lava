#pragma once

namespace lava::sill {
    class GameEntity;
}

namespace lava::sill {
    /// Interface for components.
    class IComponent {
    public:
        IComponent(GameEntity& entity)
            : m_entity(entity)
        {
        }
        virtual ~IComponent() = default;

        GameEntity& entity() { return m_entity; }
        const GameEntity& entity() const { return m_entity; }

        /// Called regularly, dt is expressed in seconds and constant between frames.
        virtual void update(float /* dt */) {}

        /// Called on a frame basis.
        virtual void updateFrame() {}

        // To be implemented
        // static std::string hrid();

    protected:
        GameEntity& m_entity;
    };
}
