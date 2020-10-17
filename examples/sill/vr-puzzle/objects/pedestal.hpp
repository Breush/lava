#pragma once

#include "./object.hpp"

class Brick;

class Pedestal final : public Object {
public:
    struct BrickInfo {
        // @note :UnconsolidatedId After deserialization and before call to consolidateReferences(),
        // unconsolidatedBrickId is the only valid value in this struct.
        // During the game, it might become wrong and should not be used.
        uint32_t unconsolidatedBrickId = -1u;
        Brick* brick = nullptr;

        lava::sill::Entity* arm = nullptr;
        float rotation1 = 0.f;
        float rotation2 = 0.f;
    };

public:
    Pedestal(GameState& gameState);
    void clear(bool removeFromLevel = true) final;

    void unserialize(const nlohmann::json& data) final;
    nlohmann::json serialize() const final;
    void consolidateReferences() final;
    void mutateBeforeDuplication(nlohmann::json& data) final;

    // Editor controls
    void uiWidgets(std::vector<UiWidget>& widgets) final;

    const std::string& substanceRevealNeeded() const { return m_substanceRevealNeeded; }
    void substanceRevealNeeded(const std::string& substanceRevealNeeded) { m_substanceRevealNeeded = substanceRevealNeeded; }

    bool powered() const { return m_powered; }
    void powered(bool powered);

    // A brick informs us that it's stored status changed.
    void brickStoredChanged(Brick& brick);

    const std::vector<BrickInfo>& brickInfos() const { return m_brickInfos; }
    void addBrick(Brick& brick);

protected:
    void updateBricksArms();

private:
    std::string m_substanceRevealNeeded = "wood";
    bool m_powered = false;

    lava::sill::Entity* m_bricksRoot = nullptr;
    std::vector<BrickInfo> m_brickInfos;
};
