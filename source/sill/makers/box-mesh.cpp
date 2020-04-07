#include <lava/sill/makers/box-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava::sill;

std::function<void(MeshComponent&)> makers::boxMeshMaker(float sidesLength, BoxMeshOptions options)
{
    return makers::boxMeshMaker({sidesLength, sidesLength, sidesLength}, options);
}

std::function<void(MeshComponent&)> makers::boxMeshMaker(const glm::vec3& extent, BoxMeshOptions options)
{
    const auto halfExtent = extent / 2.f;

    // Offset for vertices base on requested origin
    glm::vec3 verticesOffset = options.offset;
    if (options.origin == BoxOrigin::Bottom) {
        verticesOffset -= glm::vec3(0, 0, extent.z / 2.f);
    }

    // Positions
    std::vector<glm::vec3> positions = {
        // Bottom
        verticesOffset + glm::vec3(halfExtent.x, -halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, -halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(halfExtent.x, halfExtent.y, -halfExtent.z),
        // Top
        verticesOffset + glm::vec3(halfExtent.x, -halfExtent.y, halfExtent.z),
        verticesOffset + glm::vec3(halfExtent.x, halfExtent.y, halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, halfExtent.y, halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, -halfExtent.y, halfExtent.z),
        // Left
        verticesOffset + glm::vec3(halfExtent.x, halfExtent.y, halfExtent.z),
        verticesOffset + glm::vec3(halfExtent.x, halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, halfExtent.y, halfExtent.z),
        // Right
        verticesOffset + glm::vec3(-halfExtent.x, -halfExtent.y, halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, -halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(halfExtent.x, -halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(halfExtent.x, -halfExtent.y, halfExtent.z),
        // Front
        verticesOffset + glm::vec3(halfExtent.x, -halfExtent.y, halfExtent.z),
        verticesOffset + glm::vec3(halfExtent.x, -halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(halfExtent.x, halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(halfExtent.x, halfExtent.y, halfExtent.z),
        // Back
        verticesOffset + glm::vec3(-halfExtent.x, halfExtent.y, halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, -halfExtent.y, -halfExtent.z),
        verticesOffset + glm::vec3(-halfExtent.x, -halfExtent.y, halfExtent.z),
    };

    // Normals (flat shading)
    std::vector<glm::vec3> normals = {
        // Bottom
        {0.f, 0.f, -1.f},
        {0.f, 0.f, -1.f},
        {0.f, 0.f, -1.f},
        {0.f, 0.f, -1.f},
        // Top
        {0.f, 0.f, 1.f},
        {0.f, 0.f, 1.f},
        {0.f, 0.f, 1.f},
        {0.f, 0.f, 1.f},
        // Left
        {0.f, 1.f, 0.f},
        {0.f, 1.f, 0.f},
        {0.f, 1.f, 0.f},
        {0.f, 1.f, 0.f},
        // Right
        {0.f, -1.f, 0.f},
        {0.f, -1.f, 0.f},
        {0.f, -1.f, 0.f},
        {0.f, -1.f, 0.f},
        // Front
        {1.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
        // Back
        {-1.f, 0.f, 0.f},
        {-1.f, 0.f, 0.f},
        {-1.f, 0.f, 0.f},
        {-1.f, 0.f, 0.f},
    };

    // UVs
    std::vector<glm::vec2> uvs;
    switch (options.coordinatesSystem) {
    case BoxCoordinatesSystem::Box2x3: {
        uvs = {
            // Bottom
            {2.f / 3.f, 0.5f},
            {1.f, 0.5f},
            {1.f, 0.f},
            {2.f / 3.f, 0.f},
            // Top
            {0.f, 0.f},
            {0.f, 0.5f},
            {1.f / 3.f, 0.5f},
            {1.f / 3.f, 0.f},
            // Left
            {0.f, 0.5f},
            {0.f, 1.f},
            {1.f / 3.f, 1.f},
            {1.f / 3.f, 0.5f},
            // Right
            {2.f / 3.f, 0.5f},
            {2.f / 3.f, 1.f},
            {1.f, 1.f},
            {1.f, 0.5f},
            // Front
            {1.f / 3.f, 0.5f},
            {2.f / 3.f, 0.5f},
            {2.f / 3.f, 0.f},
            {1.f / 3.f, 0.f},
            // Back
            {1.f / 3.f, 0.5f},
            {1.f / 3.f, 1.f},
            {2.f / 3.f, 1.f},
            {2.f / 3.f, 0.5f},
        };
        break;
    }
    case BoxCoordinatesSystem::Unknown:
        // Nothing to do
        break;
    }

    // Indices
    std::vector<uint16_t> indices;
    indices.reserve(6u * positions.size() / 4u);
    for (auto i = 0u; i < positions.size(); i += 4u) {
        indices.emplace_back(i);
        indices.emplace_back(i + 1u);
        indices.emplace_back(i + 2u);
        indices.emplace_back(i + 2u);
        indices.emplace_back(i + 3u);
        indices.emplace_back(i);
    }

    // Correct siding
    if (options.siding == BoxSiding::In) {
        for (auto i = 0u; i < 6u; i++) {
            std::swap(indices[6u * i], indices[6u * i + 1u]);
            std::swap(indices[6u * i + 3u], indices[6u * i + 4u]);
        }
    }

    return [=](MeshComponent& meshComponent) {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        // Apply the geometry
        auto meshGroup = std::make_unique<MeshGroup>(meshComponent.entity().engine());
        auto& primitive = meshGroup->addPrimitive();
        primitive.verticesCount(positions.size());
        primitive.verticesPositions(positions);
        primitive.verticesNormals(normals);
        primitive.verticesUvs(uvs);
        primitive.indices(indices);

        std::vector<MeshNode> nodes(1u);
        nodes[0u].meshGroup = std::move(meshGroup);
        meshComponent.nodes(std::move(nodes));
    };
}
