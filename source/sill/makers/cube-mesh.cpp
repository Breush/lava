#include <lava/sill/makers/cube-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>

using namespace lava::sill;

std::function<void(MeshComponent& mesh)> makers::cubeMeshMaker(float sideLength, CubeMeshOptions options)
{
    return [sideLength, options](MeshComponent& mesh) {
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
            // Front
            {halfSideLength, halfSideLength, halfSideLength},
            {halfSideLength, halfSideLength, -halfSideLength},
            {-halfSideLength, halfSideLength, -halfSideLength},
            {-halfSideLength, halfSideLength, halfSideLength},
            // Back
            {-halfSideLength, -halfSideLength, halfSideLength},
            {-halfSideLength, -halfSideLength, -halfSideLength},
            {halfSideLength, -halfSideLength, -halfSideLength},
            {halfSideLength, -halfSideLength, halfSideLength},
            // Left
            {halfSideLength, -halfSideLength, halfSideLength},
            {halfSideLength, -halfSideLength, -halfSideLength},
            {halfSideLength, halfSideLength, -halfSideLength},
            {halfSideLength, halfSideLength, halfSideLength},
            // Right
            {-halfSideLength, halfSideLength, halfSideLength},
            {-halfSideLength, halfSideLength, -halfSideLength},
            {-halfSideLength, -halfSideLength, -halfSideLength},
            {-halfSideLength, -halfSideLength, halfSideLength},
        };

        // Normals
        const float radius = glm::normalize(positions[0u]).length();
        std::vector<glm::vec3> normals;
        normals.reserve(positions.size());

        // @todo Allow flat shading
        for (auto& position : positions) {
            normals.emplace_back(position / radius);
        }

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
                // Front
                {0.f, 0.5f},
                {0.f, 1.f},
                {1.f / 3.f, 1.f},
                {1.f / 3.f, 0.5f},
                // Back
                {2.f / 3.f, 0.5f},
                {2.f / 3.f, 1.f},
                {1.f, 1.f},
                {1.f, 0.5f},
                // Left
                {1.f / 3.f, 0.5f},
                {2.f / 3.f, 0.5f},
                {2.f / 3.f, 0.f},
                {1.f / 3.f, 0.f},
                // Right
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

        mesh.verticesCount(positions.size());
        mesh.verticesPositions(positions);
        mesh.verticesNormals(normals);
        mesh.verticesUvs(uvs);
        mesh.indices(indices);
    };
}
