#include <lava/sill/makers/cube-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>

using namespace lava::sill;

std::function<void(MeshComponent& mesh)> makers::cubeMeshMaker(float sideLength)
{
    return [sideLength](MeshComponent& mesh) {
        const auto halfSideLength = sideLength / 2.f;

        // Positions
        std::vector<glm::vec3> positions = {
            {-halfSideLength, -halfSideLength, -halfSideLength}, {-halfSideLength, halfSideLength, -halfSideLength},
            {halfSideLength, halfSideLength, -halfSideLength},   {halfSideLength, -halfSideLength, -halfSideLength},
            {-halfSideLength, -halfSideLength, halfSideLength},  {-halfSideLength, halfSideLength, halfSideLength},
            {halfSideLength, halfSideLength, halfSideLength},    {halfSideLength, -halfSideLength, halfSideLength},

        };

        // Normals
        const float radius = glm::normalize(positions[0u]).length();
        std::vector<glm::vec3> normals;
        normals.reserve(8u);

        for (auto& position : positions) {
            normals.emplace_back(position / radius);
        }

        // Indices
        std::vector<uint16_t> indices = {
            0u, 1u, 2u, 2u, 3u, 0u, // Bottom
            5u, 4u, 6u, 7u, 6u, 4u, // Top
            2u, 1u, 6u, 5u, 6u, 1u, // Front
            0u, 3u, 4u, 7u, 4u, 3u, // Back
            1u, 0u, 4u, 4u, 5u, 1u, // Right
            3u, 2u, 7u, 6u, 7u, 2u, // Left
        };

        mesh.verticesCount(positions.size());
        mesh.verticesPositions(positions);
        mesh.verticesNormals(normals);
        mesh.indices(indices);
    };
}
