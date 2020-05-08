#pragma once

#include "./generic.hpp"

class Brick;

class Pedestal final : public Generic {
public:
    Pedestal(GameState& gameState);
    void clear(bool removeFromLevel = true) final;

    void unserialize(const nlohmann::json& data) final;
    nlohmann::json serialize() const final;
    void consolidateReferences() final;

    const std::string& material() const { return m_material; }

    bool powered() const { return m_powered; }
    void powered(bool powered);

    // A brick informs us that it's stored status changed.
    void brickStoredChanged(Brick& brick);

    void addBrick(Brick& brick);

protected:
    void updateBricksArms();

protected:
    struct BrickInfo {
        // @note :UnconsolidatedId After deserialization and before call to consolidateReferences(),
        // unconsolidatedBrickId is the only valid value in this struct.
        // During the game, it might become wrong and should not be used.
        uint32_t unconsolidatedBrickId = -1u;
        Brick* brick = nullptr;

        lava::sill::GameEntity* arm = nullptr;
        float rotation1 = 0.f;
        float rotation2 = 0.f;
    };

private:
    std::string m_material = "wood";
    bool m_powered = false;

    lava::sill::GameEntity* m_bricksRoot = nullptr;
    std::vector<BrickInfo> m_brickInfos;
};
