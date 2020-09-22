#pragma once

#include <lava/sill.hpp>
#include <nlohmann/json.hpp>

struct GameState;

class Frame {
public:
    Frame(GameState& gameState);
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

    /// Create a frame.
    static Frame& make(GameState& gameState);

    /// Prepare the frame to be removed.
    /// The destructor does not destroy anything
    /// so that shutting down the application is fast enough.
    virtual void clear(bool removeFromLevel = true);

    const std::string& name() const { return m_name; }
    void name(const std::string& name) { m_name = name; }

    const lava::sill::EntityFrame& entityFrame() const { return *m_entityFrame; }
    lava::sill::EntityFrame& entityFrame() { return *m_entityFrame; }
    void entityFrame(lava::sill::EntityFrame& entityFrame) { m_entityFrame = &entityFrame; }

    const lava::sill::MeshFrameComponent& mesh() const { return m_entityFrame->get<lava::sill::MeshFrameComponent>(); };
    lava::sill::MeshFrameComponent& mesh() { return m_entityFrame->get<lava::sill::MeshFrameComponent>(); };

protected:
    GameState& m_gameState;
    lava::sill::EntityFrame* m_entityFrame = nullptr;

    std::string m_name;
};

uint32_t findFrameIndex(GameState& gameState, const lava::sill::EntityFrame& entityFrame);
