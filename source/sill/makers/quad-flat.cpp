#include <lava/sill/makers/quad-flat.hpp>

#include <lava/sill/components/flat-component.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava;
using namespace lava::sill;

std::function<FlatNode&(FlatComponent&)> makers::quadFlatMaker(float sidesLength)
{
    return makers::quadFlatMaker({sidesLength, sidesLength});
}

std::function<FlatNode&(FlatComponent&)> makers::quadFlatMaker(const glm::vec2& extent)
{
    const auto halfExtent = extent / 2.f;

    // Positions
    std::vector<glm::vec2> positions = {
        {-halfExtent.x, -halfExtent.y},
        {-halfExtent.x, halfExtent.y},
        {halfExtent.x, -halfExtent.y},
        {halfExtent.x, halfExtent.y},
    };

    // UVs
    std::vector<glm::vec2> uvs = {
        {0.f, 0.f},
        {0.f, 1.f},
        {1.f, 0.f},
        {1.f, 1.f},
    };

    // Indices
    std::vector<uint16_t> indices = {
        0u, 2u, 3u,
        3u, 1u, 0u,
    };

    return [positions, uvs, indices](FlatComponent& flatComponent) -> FlatNode& {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        // Apply the geometry
        auto& node = flatComponent.addNode();
        node.flatGroup = std::make_unique<FlatGroup>(flatComponent.entity().engine());
        auto& primitive = node.flatGroup->addPrimitive();
        primitive.verticesCount(positions.size());
        primitive.verticesPositions(positions);
        primitive.verticesUvs(uvs);
        primitive.indices(indices);

        return node;
    };
}
