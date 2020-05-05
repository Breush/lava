#pragma once

#include "./generic.hpp"

class Brick;

class Pedestal final : public Generic {
public:
    Pedestal(GameState& gameState);
    void clear(bool removeFromLevel = true) final;

    void unserialize(const nlohmann::json& data);
    nlohmann::json serialize() const;
    void consolidateReferences();

    const std::string& material() const { return m_material; }

    bool powered() const { return m_powered; }
    void powered(bool powered);

protected:
    struct BrickInfo {
        // @note After deserialization and before call to consolidateReferences(),
        // unconsolidatedBrickId is the only valid value in this struct.
        // During the game, it might become wrong and should not be used.
        uint32_t unconsolidatedBrickId = -1u;
        Brick* brick = nullptr;

        float rotation1 = 0.f;
        float rotation2 = 0.f;
    };

private:
    std::string m_material;
    bool m_powered = false;

    lava::sill::GameEntity* m_bricksRoot = nullptr;
    std::vector<BrickInfo> m_brickInfos;
};
