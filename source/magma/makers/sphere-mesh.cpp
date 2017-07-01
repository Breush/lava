#include <lava/magma/makers/sphere-mesh.hpp>

#include <algorithm>
#include <lava/chamber/math.hpp>
#include <lava/magma/meshes/mesh.hpp>

namespace {
    void addCirclePoints(std::vector<glm::vec3>& points, const uint32_t tessellation, const float radius, const float height)
    {
        const auto step = lava::chamber::math::TWO_PI / tessellation;
        const auto cStep = lava::chamber::math::cos(step);
        const auto sStep = lava::chamber::math::sin(step);

        glm::vec3 point{radius, 0.f, height};
        for (auto j = 0u; j < tessellation; ++j) {
            points.emplace_back(point);
            const auto px = point.x;
            point.x = px * cStep - point.y * sStep;
            point.y = px * sStep + point.y * cStep;
        }
    }

    void addPoleStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex, uint16_t poleIndex,
                      bool reverse)
    {
        for (auto j = 0u, i = tessellation - 1u; j < tessellation; i = j++) {
            uint16_t d0 = startIndex + i;
            uint16_t d1 = startIndex + j;
            if (reverse) std::swap(d0, d1);
            indices.emplace_back(d0);
            indices.emplace_back(d1);
            indices.emplace_back(poleIndex);
        }
    }

    void addRowStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex)
    {
        for (auto j = 0u, i = tessellation - 1u; j < tessellation; i = j++) {
            uint16_t d0 = startIndex + i;
            uint16_t d1 = startIndex + j;
            uint16_t u0 = d0 + tessellation;
            uint16_t u1 = d1 + tessellation;
            indices.emplace_back(d0);
            indices.emplace_back(d1);
            indices.emplace_back(u1);
            indices.emplace_back(u1);
            indices.emplace_back(u0);
            indices.emplace_back(d0);
        }
    }
}

using namespace lava;
using namespace lava::chamber;

std::function<void(Mesh& mesh)> makers::sphereMeshMaker(uint32_t tessellation, float radius)
{
    return [tessellation, radius](Mesh& mesh) {
        // @todo Reserve those
        std::vector<glm::vec3> positions;
        std::vector<uint16_t> indices;

        // South pole
        addPoleStrip(indices, tessellation, 1u, 0u, true);
        positions.emplace_back(glm::vec3{0.f, 0.f, -radius});

        // Main strips
        const auto latitudeStep = math::PI / (tessellation - 1u);
        auto latitude = -math::PI_OVER_TWO + latitudeStep;
        for (auto i = 1u; i < tessellation - 2u; ++i) {
            // Adding the strip indices of that circle and the one over it.
            addRowStrip(indices, tessellation, positions.size());

            // This is a circle at a fixed latitude.
            const auto rclat = radius * math::cos(latitude);
            const auto rslat = radius * math::sin(latitude);
            addCirclePoints(positions, tessellation, rclat, rslat);
            latitude += latitudeStep;
        }

        // Last row of points
        const auto rclat = radius * math::cos(latitude);
        const auto rslat = radius * math::sin(latitude);
        addCirclePoints(positions, tessellation, rclat, rslat);

        // North pole
        const uint16_t northPoleIndex = positions.size();
        const uint16_t indexStart = northPoleIndex - tessellation;
        addPoleStrip(indices, tessellation, indexStart, northPoleIndex, false);
        positions.emplace_back(glm::vec3{0.f, 0.f, radius});

        // Normals
        std::vector<glm::vec3> normals;
        normals.reserve(positions.size());
        for (const auto& position : positions) {
            normals.emplace_back(position / radius);
        }

        mesh.verticesCount(positions.size());
        mesh.verticesPositions(positions);
        mesh.verticesNormals(normals);
        mesh.indices(indices);
    };
}
