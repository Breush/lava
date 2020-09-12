#include "./brick.hpp"

#include <algorithm>

#include "../game-state.hpp"
#include "../serializer.hpp"
#include "./pedestal.hpp"

using namespace lava;

Brick::Brick(GameState& gameState)
    : Object(gameState)
{
    m_gameState.level.bricks.emplace_back(this);

    m_entity->name("brick");
    m_entity->ensure<sill::TransformComponent>();
    m_entity->ensure<sill::AnimationComponent>();
    m_entity->ensure<sill::MeshComponent>();

    m_unconsolidatedBlocks.emplace_back(0, 0);
}

void Brick::clear(bool removeFromLevel)
{
    if (removeFromLevel) {
        auto brickIt = std::find(m_gameState.level.bricks.begin(), m_gameState.level.bricks.end(), this);
        m_gameState.level.bricks.erase(brickIt);
    }

    // @note Keep last, this destroys us!
    Object::clear(removeFromLevel);
}

void Brick::unserialize(const nlohmann::json& data)
{
    m_unconsolidatedBlocks.clear();
    for (auto& blockData : data["blocks"]) {
        m_unconsolidatedBlocks.emplace_back(unserializeIvec2(blockData));
    }

    color(unserializeVec3(data["color"]));
    fixed(data["fixed"]);
    stored(data["stored"]);
    baseRotationLevel(data["rotationLevel"]);

    auto& snapPanelData = data["snapPanel"];
    if (!snapPanelData.is_null()) {
        m_snapPanelUnconsolidatedId = snapPanelData;
        m_snapCoordinates = unserializeUvec2(data["snapCoordinates"]);
    }
}

nlohmann::json Brick::serialize() const
{
    nlohmann::json data = {
        {"blocks", nlohmann::json::array()},
        {"color", ::serialize(m_color)},
        {"fixed", m_fixed},
        {"stored", m_stored},
        {"rotationLevel", m_rotationLevel},
        {"snapPanel", {}},
    };

    for (const auto& block : m_blocks) {
        data["blocks"].emplace_back(::serialize(block.nonRotatedCoordinates));
    }

    if (m_snapPanel != nullptr) {
        data["snapPanel"] = findPanelIndex(m_gameState, m_snapPanel->entity());
        data["snapCoordinates"] = ::serialize(m_snapCoordinates);
    }

    return data;
}

void Brick::mutateBeforeDuplication(nlohmann::json& data)
{
    // @note When duplicating an already stored brick,
    // we don't attach it back to the same pedestal.
    data["stored"] = false;
}

void Brick::consolidateReferences()
{
    blocks(m_unconsolidatedBlocks);

    if (m_snapPanelUnconsolidatedId != -1u) {
        auto& panel = *m_gameState.level.panels[m_snapPanelUnconsolidatedId];
        snap(panel, m_snapCoordinates);
    }
}

float Brick::halfSpan() const
{
    glm::ivec2 minBlock{0};
    glm::ivec2 maxBlock{0};
    for (const auto& block : m_blocks) {
        minBlock = glm::max(minBlock, block.nonRotatedCoordinates);
        maxBlock = glm::max(maxBlock, block.nonRotatedCoordinates);
    }

    auto brickHalfSpan = std::max(blockExtent.x * (std::max(maxBlock.x, std::abs(minBlock.x)) + 1),
                                  blockExtent.y * (std::max(maxBlock.y, std::abs(minBlock.y)) + 1));

    return brickHalfSpan;
}

void Brick::uiWidgets(std::vector<UiWidget>& widgets)
{
    Object::uiWidgets(widgets);

    widgets.emplace_back("fixed", "fixed",
        [this]() -> bool { return fixed(); },
        [this](bool fixed) { this->fixed(fixed); }
    );
}

// -----

void Brick::blocks(const std::vector<glm::ivec2>& blocks)
{
    constexpr const auto blockScaling = 0.75f;

    // Clean
    m_blocks.resize(blocks.size());
    mesh().removeNodes();
    m_bricksMeshNodeIndex = -1u;
    
    // Create blocks
    static auto blockMaker = sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-brick.glb");
    glm::mat4 bricksRootTransform(1.f);
    for (auto i = 0u; i < blocks.size(); ++i) {
        m_blocks[i].nonRotatedCoordinates = blocks[i];

        // Make block and instance it
        sill::MeshNode* rootNode = nullptr;
        if (i == 0u) {
            rootNode = &blockMaker(mesh());
            bricksRootTransform = rootNode->transform();
            m_bricksMeshNodeIndex = mesh().nodes().size() - 1u;
        } else {
            rootNode = &mesh().addInstancedNode(0u);
        }

        auto transform = glm::mat4(1.f);
        transform = glm::translate(transform, {glm::vec2(blocks[i]) * glm::vec2(blockExtent), 0.f});
        transform = glm::scale(transform, {blockScaling, blockScaling, 1.f});
        rootNode->transform(transform * bricksRootTransform);
    }

    mesh().path(std::string()); // So that the mesh component is not serialized.

    // Update blocks
    updateBlocksColor();
    updateBlocksFromRotationLevel();
}

void Brick::addBlockV(int32_t x, bool positive)
{
    std::vector<glm::ivec2> blocks;

    int32_t y = 0;
    for (const auto& block : m_blocks) {
        blocks.emplace_back(block.nonRotatedCoordinates);
        if (block.nonRotatedCoordinates.x != x) continue;
        if (positive && block.nonRotatedCoordinates.y >= y) {
            y = block.nonRotatedCoordinates.y + 1;
        }
        else if (!positive && block.nonRotatedCoordinates.y <= y) {
            y = block.nonRotatedCoordinates.y - 1;
        }
    }

    blocks.emplace_back(x, y);

    this->blocks(blocks);
}

void Brick::addBlockH(int32_t y, bool positive)
{
    std::vector<glm::ivec2> blocks;

    int32_t x = 0;
    for (const auto& block : m_blocks) {
        blocks.emplace_back(block.nonRotatedCoordinates);
        if (block.nonRotatedCoordinates.y != y) continue;
        if (positive && block.nonRotatedCoordinates.x >= x) {
            x = block.nonRotatedCoordinates.x + 1;
        }
        else if (!positive && block.nonRotatedCoordinates.x <= x) {
            x = block.nonRotatedCoordinates.x - 1;
        }
    }

    blocks.emplace_back(x, y);

    this->blocks(blocks);
}

bool Brick::userInteractionAllowed() const
{
    return !m_fixed;
}

void Brick::color(const glm::vec3& color)
{
    m_color = color;

    updateBlocksColor();
}

void Brick::selectionHighlighted(bool selectionHighlighted) {
    if (m_selectionHighlighted == selectionHighlighted) return;
    m_selectionHighlighted = selectionHighlighted;

    updateBlocksColor();
}

void Brick::errorHighlighted(bool errorHighlighted) {
    if (m_errorHighlighted == errorHighlighted) return;
    m_errorHighlighted = errorHighlighted;

    updateBlocksColor();
}

void Brick::fixed(bool fixed) {
    if (m_fixed == fixed) return;
    m_fixed = fixed;

    updateBlocksColor();
}

void Brick::stored(bool stored)
{
    if (m_stored == stored) return;
    m_stored = stored;

    if (m_pedestal) {
        m_pedestal->brickStoredChanged(*this);
    }
}

void Brick::unsnap()
{
    if (m_snapPanel == nullptr) return;

    auto snapPanel = m_snapPanel;
    m_snapPanel = nullptr;

    snapPanel->snappedBricksChanged();
}

void Brick::snap(Panel& panel, const glm::uvec2& snapCoordinates)
{
    if (m_snapPanel == &panel && m_snapCoordinates == snapCoordinates) return;

    m_snapPanel = &panel;
    m_snapCoordinates = snapCoordinates;
    m_snapPanel->snappedBricksChanged();
}

uint32_t Brick::incrementBaseRotationLevel()
{
    baseRotationLevel((m_baseRotationLevel + 1u) % 4u);
    return m_baseRotationLevel;
}

void Brick::baseRotationLevel(uint32_t baseRotationLevel)
{
    if (m_baseRotationLevel == baseRotationLevel) return;
    m_baseRotationLevel = baseRotationLevel;
    m_rotationLevel = m_baseRotationLevel + m_extraRotationLevel;
    updateBlocksFromRotationLevel();
}

void Brick::extraRotationLevel(uint32_t extraRotationLevel)
{
    if (m_extraRotationLevel == extraRotationLevel) return;
    m_extraRotationLevel = extraRotationLevel;
    m_rotationLevel = m_baseRotationLevel + m_extraRotationLevel;
    updateBlocksFromRotationLevel();
}

// Internal

void Brick::updateBlocksColor()
{
    auto color = m_color;

    if (m_fixed) {
        color = glm::vec3(0.8f);
    }
    else if (m_errorHighlighted) {
        color = glm::mix(color, glm::vec3(0.95f, 0.2f, 0.2f), 0.75f);
    }
    else if (m_selectionHighlighted) {
        color = glm::mix(color, glm::vec3(0.4f, 0.4f, 0.95f), 0.75f);
    }

    if (m_bricksMeshNodeIndex != -1u) {
        mesh().material(m_bricksMeshNodeIndex, 0u)->set("albedoColor", color);
    }
}

void Brick::updateBlocksFromRotationLevel()
{
    for (auto& block : m_blocks) {
        block.coordinates = block.nonRotatedCoordinates;

        // X <= -Y and Y <=  X is a 90° clockwise rotation
        for (auto k = 0u; k < m_rotationLevel; ++k) {
            std::swap(block.coordinates.x, block.coordinates.y);
            block.coordinates.x = -block.coordinates.x;
        }
    }
}

// -----

Brick* findBrick(GameState& gameState, const sill::GameEntity& entity)
{
    for (auto brick : gameState.level.bricks) {
        if (&brick->entity() == &entity) {
            return brick;
        }
    }

    return nullptr;
}

uint32_t findBrickIndex(GameState& gameState, const sill::GameEntity& entity)
{
    for (auto i = 0u; i < gameState.level.bricks.size(); ++i) {
        auto& brick = *gameState.level.bricks[i];
        if (&brick.entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
