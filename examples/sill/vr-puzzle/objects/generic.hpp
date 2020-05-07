#pragma once

#include "./object.hpp"

#include <nlohmann/json.hpp>

class Generic : public Object {
public:
    Generic(GameState& gameState);
    virtual ~Generic() = default;
    virtual void clear(bool removeFromLevel = true);

    const std::string& name() const { return m_name; }
    void name(const std::string& name) { m_name = name; }
    const std::string& kind() const { return m_kind; }
    template<class T> T& as() { return dynamic_cast<T&>(*this); }

    virtual nlohmann::json serialize() const { return nullptr; }
    virtual void unserialize(const nlohmann::json& /* data */) {}
    virtual void consolidateReferences() {}

    static Generic& make(GameState& gameState, const std::string& kind);

    // @todo I find it strange having that here...
    // This is not completely cross-generics.
    // We better have an other class for that with own serialization data.
    bool walkable() const { return m_walkable; }
    void walkable(bool walkable) { m_walkable = walkable; }

protected:
    std::string m_name;
    std::string m_kind;

    bool m_walkable = true;
};

Generic* findGenericByName(GameState& gameState, const std::string& name);
uint32_t findGenericIndex(GameState& gameState, const lava::sill::GameEntity& entity);
