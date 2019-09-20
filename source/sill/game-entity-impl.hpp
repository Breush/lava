#pragma once

#include <lava/sill/game-entity.hpp>

#include <lava/sill/components/i-component.hpp>
#include <lava/sill/game-engine.hpp>

namespace lava::sill {
    class GameEntity::Impl {
    public:
        Impl(GameEntity& entity, GameEngine& engine);
        ~Impl();

        GameEntity& entity() { return m_entity; }
        const GameEntity& entity() const { return m_entity; }

        // GameEntity hierarchy
        GameEntity* parent() { return m_parent; }
        const GameEntity* parent() const { return m_parent; }
        void parent(GameEntity& parent, bool updateParent) { this->parent(&parent, updateParent); }
        void parent(GameEntity* parent, bool updateParent);
        void addChild(GameEntity& child, bool updateChild);
        void forgetChild(GameEntity& child, bool updateChild);
        const std::vector<GameEntity*>& children() const { return m_children; }

        // GameEntity components
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
        GameEntity& m_entity;
        GameEngine::Impl& m_engine;

        // Storage
        GameEntity* m_parent = nullptr; // Keep nullptr to be top-level.
        std::vector<GameEntity*> m_children;
        std::unordered_map<std::string, std::unique_ptr<IComponent>> m_components;
        std::unordered_map<std::string, std::unique_ptr<IComponent>> m_pendingAddedComponents;
    };
}
