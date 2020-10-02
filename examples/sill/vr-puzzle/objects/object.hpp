#pragma once

#include <lava/sill.hpp>
#include <lava/magma.hpp>
#include <nlohmann/json.hpp>

#include "../ui-widget.hpp"

struct GameState;
class Frame;

class Object {
public:
    Object(GameState& gameState);
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    virtual ~Object() = default;

    const std::string& kind() const { return m_kind; }

    Frame* frame() const { return m_frame; }
    void frame(Frame* frame) { m_frame = frame; }
    void frame(Frame& frame) { m_frame = &frame; }

    /// Create an object of a certain kind.
    static Object& make(GameState& gameState, const std::string& kind);
    template<class T> T& as() { return dynamic_cast<T&>(*this); }

    /// Prepare the object to be removed.
    /// The destructor does not destroy anything
    /// so that shutting down the application is fast enough.
    virtual void clear(bool removeFromLevel = true);

    /// Control serialization.
    virtual nlohmann::json serialize() const { return nullptr; }
    virtual void unserialize(const nlohmann::json& /* data */) {}
    virtual void consolidateReferences() {}
    virtual void mutateBeforeDuplication(nlohmann::json& /* data */) {}

    // Editor controls
    virtual void uiWidgets(std::vector<UiWidget>& widgets);
    virtual void editorOnClicked(const glm::vec3& /* hitPoint */) {}
    virtual const glm::vec3& editorOrigin() const {
        return transform().worldTransform().translation;
    }
    virtual void editorTranslate(const glm::vec3& translation) {
        transform().worldTranslate(translation);
    }

    /// The offset by which duplication will occur on x axis.
    virtual float halfSpan() const {
        return mesh().boundingSphere().radius;
    }

    const std::string& name() const { return m_name; }
    void name(const std::string& name) { m_name = name; }

    const lava::sill::Entity& entity() const { return *m_entity; }
    lava::sill::Entity& entity() { return *m_entity; }
    void entity(lava::sill::Entity& entity) { m_entity = &entity; }

    const lava::sill::AnimationComponent& animation() const { return m_entity->get<lava::sill::AnimationComponent>(); };
    lava::sill::AnimationComponent& animation() { return m_entity->get<lava::sill::AnimationComponent>(); };
    const lava::sill::TransformComponent& transform() const { return m_entity->get<lava::sill::TransformComponent>(); };
    lava::sill::TransformComponent& transform() { return m_entity->get<lava::sill::TransformComponent>(); };
    const lava::sill::MeshComponent& mesh() const { return m_entity->get<lava::sill::MeshComponent>(); };
    lava::sill::MeshComponent& mesh() { return m_entity->get<lava::sill::MeshComponent>(); };

protected:
    GameState& m_gameState;
    lava::sill::Entity* m_entity = nullptr;
    Frame* m_frame = nullptr;

    std::string m_kind;
    std::string m_name;
};

Object* findObject(GameState& gameState, const lava::sill::Entity* entity);
Object* findObject(GameState& gameState, const lava::sill::Entity& entity);
