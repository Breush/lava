#pragma once

#include <lava/sill.hpp>
#include <lava/magma.hpp>

struct GameState;

class Object {
public:
    Object(GameState& gameState);
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    virtual ~Object() = default;

    /// Prepare the object to be removed.
    /// The destructor does not destroy anything
    /// so that shutting down the application is fast enough.
    virtual void clear(bool removeFromLevel = true);

    /// The offset by which duplication will occur on x axis.
    virtual float halfSpan() const {
        return mesh().boundingSphere().radius;
    }

    const lava::sill::GameEntity& entity() const { return *m_entity; }
    lava::sill::GameEntity& entity() { return *m_entity; }
    void entity(lava::sill::GameEntity& entity) { m_entity = &entity; }

    const lava::sill::AnimationComponent& animation() const { return m_entity->get<lava::sill::AnimationComponent>(); };
    lava::sill::AnimationComponent& animation() { return m_entity->get<lava::sill::AnimationComponent>(); };
    const lava::sill::TransformComponent& transform() const { return m_entity->get<lava::sill::TransformComponent>(); };
    lava::sill::TransformComponent& transform() { return m_entity->get<lava::sill::TransformComponent>(); };
    const lava::sill::MeshComponent& mesh() const { return m_entity->get<lava::sill::MeshComponent>(); };
    lava::sill::MeshComponent& mesh() { return m_entity->get<lava::sill::MeshComponent>(); };

protected:
    GameState& m_gameState;
    lava::sill::GameEntity* m_entity = nullptr;
};

Object* findObject(GameState& gameState, const lava::sill::GameEntity* entity);
Object* findObject(GameState& gameState, const lava::sill::GameEntity& entity);
