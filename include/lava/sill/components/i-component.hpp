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

        /// Called once per frame, respecting components dependencies.
        /// dt is expressed in seconds.
        /// @fixme This is internal, and should be seen from user API.
        virtual void update(float dt) = 0;

        // To be implemented
        // static std::string hrid();

    protected:
        GameEntity& m_entity;
    };
}
