#include "./environment.hpp"

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
    }

    // Puzzle bricks
    for (auto i = 0u; i < 9u; ++i) {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-brick.glb")(meshComponent);
        entity.make<sill::BoxColliderComponent>(glm::vec3{0.22, 0.22, 0.125});
        entity.make<sill::AnimationComponent>();

        gameState.draggableEntities.emplace_back(&entity);
    }
}
