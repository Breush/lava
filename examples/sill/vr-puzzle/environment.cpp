#include "./environment.hpp"

#include <sstream>

using namespace lava;

void setupEnvironment(GameState& gameState)
{
    auto& engine = *gameState.engine;

    // Ground
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker({10, 10})(meshComponent);
        entity.make<sill::PlaneColliderComponent>();
    }

    // Puzzle table
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-table.glb")(meshComponent);
        entity.get<sill::TransformComponent>().rotate({0, 0, 1}, 3.14156);

        for (auto& node : meshComponent.nodes()) {
            if (node.name == "table") continue;

            // Getting link to panel material
            if (node.name == "panel") {
                gameState.tablePanelMaterial = &node.mesh->primitive(0).material();
                continue;
            }

            // Gettings links to nodes of binding points
            std::stringstream nameStream;
            nameStream << node.name << std::endl;
            uint32_t i, j;
            nameStream >> i;
            nameStream >> j;

            if (i + 1 > gameState.tableBindingNodes.size()) gameState.tableBindingNodes.resize(i + 1);
            auto& tableBindingNodes = gameState.tableBindingNodes[i];

            if (j + 1 > tableBindingNodes.size()) tableBindingNodes.resize(j + 1);
            tableBindingNodes[j] = &node;
        }
    }
}

void loadLevel(GameState& gameState, uint32_t levelId)
{
    if (levelId != 0) return;

    auto& engine = *gameState.engine;

    /**
     * Level 0 is composed of:
     *  - 3x3 void panel
     *  - 1 L3 brick
     *  - 1 T4 brick
     *  - 1 I2 brick
     */

    // Load default look for the table panel
    {
        auto& texture = engine.make<sill::Texture>();
        texture.loadFromFile("./assets/textures/vr-puzzle/panel-3x3-base.jpg");
        gameState.tablePanelMaterial->set("albedoMap", texture);
    }

    // Puzzle individual bricks
    std::vector<sill::GameEntity*> bricks;
    auto brickMaker = sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-brick.glb");
    for (auto i = 0u; i < 9u; ++i) {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        brickMaker(meshComponent);
        bricks.emplace_back(&entity);
    }

    constexpr const glm::vec3 brickExtent = {0.22f, 0.22f, 0.125f};

    // L3 brick
    {
        auto& entity = engine.make<sill::GameEntity>();
        entity.make<sill::TransformComponent>();
        entity.make<sill::AnimationComponent>();
        // @fixme Make better colliders with some sort of PhysicsComponent
        // Moreover, we can't offset this one. Too bad.
        entity.make<sill::BoxColliderComponent>(glm::vec3{2 * brickExtent.x, 2 * brickExtent.y, brickExtent.z});

        entity.addChild(*bricks[0]);
        entity.addChild(*bricks[1]);
        entity.addChild(*bricks[2]);

        bricks[1]->get<sill::TransformComponent>().translate({brickExtent.x, 0, 0});
        bricks[2]->get<sill::TransformComponent>().translate({0, brickExtent.y, 0});

        gameState.draggableEntities.emplace_back(&entity);
    }

    // T4 brick
    {
        auto& entity = engine.make<sill::GameEntity>();
        entity.make<sill::TransformComponent>();
        entity.make<sill::AnimationComponent>();
        entity.make<sill::BoxColliderComponent>(glm::vec3{3 * brickExtent.x, 2 * brickExtent.y, brickExtent.z});

        entity.addChild(*bricks[3]);
        entity.addChild(*bricks[4]);
        entity.addChild(*bricks[5]);
        entity.addChild(*bricks[6]);

        bricks[3]->get<sill::TransformComponent>().translate({-brickExtent.x, 0, 0});
        bricks[5]->get<sill::TransformComponent>().translate({brickExtent.x, 0, 0});
        bricks[6]->get<sill::TransformComponent>().translate({0, brickExtent.y, 0});

        gameState.draggableEntities.emplace_back(&entity);
    }

    // I2 brick
    {
        auto& entity = engine.make<sill::GameEntity>();
        entity.make<sill::TransformComponent>();
        entity.make<sill::AnimationComponent>();
        entity.make<sill::BoxColliderComponent>(glm::vec3{brickExtent.x, 2 * brickExtent.y, brickExtent.z});

        entity.addChild(*bricks[7]);
        entity.addChild(*bricks[8]);

        bricks[8]->get<sill::TransformComponent>().translate({0, brickExtent.y, 0});

        gameState.draggableEntities.emplace_back(&entity);
    }
}
