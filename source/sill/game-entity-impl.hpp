#pragma once

#include <lava/sill/game-entity.hpp>

#include <lava/sill/components/i-component.hpp>
#include <lava/sill/game-engine.hpp>

namespace lava::sill {
    class GameEntity::Impl {
    public:
        Impl(GameEngine& engine);

        // GameEntity
        bool hasComponent(const std::string& hrid) const;
        IComponent& getComponent(const std::string& hrid);
        const IComponent& getComponent(const std::string& hrid) const;
        void add(const std::string& hrid, std::unique_ptr<IComponent>&& component);
        void removeComponent(const std::string& hrid);

        // Forwarded to components.
        void update(float dt);

        GameEngine::Impl& engine() { return m_engine; }
        const GameEngine::Impl& engine() const { return m_engine; }

    private:
        // References
        GameEngine::Impl& m_engine;

        // Storage
        std::unordered_map<std::string, std::unique_ptr<IComponent>> m_components;
        std::unordered_map<std::string, std::unique_ptr<IComponent>> m_pendingAddedComponents;
    };
}
