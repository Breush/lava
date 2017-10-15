#include <lava/sill/makers/plane-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>

using namespace lava::sill;

std::function<void(MeshComponent& mesh)> makers::planeMeshMaker(Extent2d dimensions)
{
    return [dimensions](MeshComponent& mesh) {
        std::vector<glm::vec3> positions(4);
        std::vector<glm::vec3> normals(4, {0.f, 0.f, 1.f});
        std::vector<glm::vec4> tangents(4, {1.f, 0.f, 0.f, 1.f});
        std::vector<uint16_t> indices = {0u, 1u, 2u, 2u, 3u, 0u};

        const auto halfWidth = dimensions.width / 2.f;
        const auto halfHeight = dimensions.height / 2.f;

        positions[0].x = -halfWidth;
        positions[0].y = -halfHeight;
        positions[1].x = halfWidth;
        positions[1].y = -halfHeight;
        positions[2].x = halfWidth;
        positions[2].y = halfHeight;
        positions[3].x = -halfWidth;
        positions[3].y = halfHeight;

        mesh.verticesCount(positions.size());
        mesh.verticesPositions(positions);
        mesh.verticesNormals(normals);
        mesh.verticesTangents(tangents);
        mesh.indices(indices);
    };
}
