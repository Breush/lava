#include <lava/sill/makers/cube-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava::sill;

std::function<void(MeshComponent&)> makers::cubeMeshMaker(float sideLength, CubeMeshOptions options)
{
    return [sideLength, options](MeshComponent& meshComponent) {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        const auto halfSideLength = sideLength / 2.f;

        // Positions
        std::vector<glm::vec3> positions = {
            // Bottom
            {halfSideLength, -halfSideLength, -halfSideLength},
            {-halfSideLength, -halfSideLength, -halfSideLength},
            {-halfSideLength, halfSideLength, -halfSideLength},
            {halfSideLength, halfSideLength, -halfSideLength},
            // Top
            {halfSideLength, -halfSideLength, halfSideLength},
            {halfSideLength, halfSideLength, halfSideLength},
            {-halfSideLength, halfSideLength, halfSideLength},
            {-halfSideLength, -halfSideLength, halfSideLength},
            // Left
            {halfSideLength, halfSideLength, halfSideLength},
            {halfSideLength, halfSideLength, -halfSideLength},
            {-halfSideLength, halfSideLength, -halfSideLength},
            {-halfSideLength, halfSideLength, halfSideLength},
            // Right
            {-halfSideLength, -halfSideLength, halfSideLength},
            {-halfSideLength, -halfSideLength, -halfSideLength},
            {halfSideLength, -halfSideLength, -halfSideLength},
            {halfSideLength, -halfSideLength, halfSideLength},
            // Front
            {halfSideLength, -halfSideLength, halfSideLength},
            {halfSideLength, -halfSideLength, -halfSideLength},
            {halfSideLength, halfSideLength, -halfSideLength},
            {halfSideLength, halfSideLength, halfSideLength},
            // Back
            {-halfSideLength, halfSideLength, halfSideLength},
            {-halfSideLength, halfSideLength, -halfSideLength},
            {-halfSideLength, -halfSideLength, -halfSideLength},
            {-halfSideLength, -halfSideLength, halfSideLength},
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
        case CubeCoordinatesSystem::BOX_2x3: {
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
        case CubeCoordinatesSystem::UNKNOWN:
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
        if (options.siding == CubeSiding::IN) {
            for (auto i = 0u; i < 6u; i++) {
                std::swap(indices[6u * i], indices[6u * i + 1u]);
                std::swap(indices[6u * i + 3u], indices[6u * i + 4u]);
            }
        }

        // Apply the geometry
        auto mesh = std::make_unique<Mesh>();
        auto& primitive = mesh->addPrimitive(meshComponent.entity().engine());
        primitive.verticesCount(positions.size());
        primitive.verticesPositions(positions);
        primitive.verticesNormals(normals);
        primitive.verticesUvs(uvs);
        primitive.indices(indices);

        std::vector<MeshNode> nodes(1u);
        nodes[0u].mesh = std::move(mesh);
        meshComponent.nodes(std::move(nodes));
    };
}
