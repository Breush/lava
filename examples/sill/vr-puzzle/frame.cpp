#include "./frame.hpp"

#include "./game-state.hpp"

#include <lava/chamber/logger.hpp>

using namespace lava;

Frame::Frame(GameState& gameState)
    : m_gameState(gameState)
{
    m_entityFrame = &gameState.engine->make<sill::EntityFrame>();
}

void Frame::clear(bool removeFromLevel)
{
    if (m_entityFrame != nullptr) {
        m_gameState.engine->remove(*m_entityFrame);
    }

    if (removeFromLevel) {
        auto frameIt = std::find_if(m_gameState.level.frames.begin(), m_gameState.level.frames.end(), [this](const std::unique_ptr<Frame>& frame) {
            return (frame.get() == this);
        });
        m_gameState.level.frames.erase(frameIt);
    }
}

Frame& Frame::make(GameState& gameState)
{
    return *gameState.level.frames.emplace_back(std::make_unique<Frame>(gameState));
}

uint32_t findFrameIndex(GameState& gameState, const sill::EntityFrame& entityFrame)
{
    for (auto i = 0u; i < gameState.level.frames.size(); ++i) {
        auto& frame = *gameState.level.frames[i];
        if (&frame.entityFrame() == &entityFrame) {
            return i;
        }
    }

    return -1u;
}
