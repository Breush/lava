#include "./mesh-tools.hpp"

using namespace lava;
using namespace lava::chamber;
using namespace lava::magma;

namespace {
    void addCirclePoints(std::vector<glm::vec3>& points, const uint32_t tessellation, const float radius, const float height)
    {
        const auto step = chamber::math::TWO_PI / tessellation;
        const auto cStep = chamber::math::cos(step);
        const auto sStep = chamber::math::sin(step);

        glm::vec3 point{radius, 0.f, height};
        for (auto j = 0u; j < tessellation; ++j) {
            points.emplace_back(point);
            const auto px = point.x;
            point.x = px * cStep - point.y * sStep;
            point.y = px * sStep + point.y * cStep;
        }

        // Last point is the first one
        points.emplace_back(radius, 0.f, height);
    }

    void addPoleStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex, uint16_t poleIndex,
                      bool reverse)
    {
        for (auto i = 0u; i < tessellation; i++) {
            uint16_t d0 = startIndex + i;
            uint16_t d1 = d0 + 1u;
            if (reverse) std::swap(d0, d1);
            indices.emplace_back(d0);
            indices.emplace_back(d1);
            indices.emplace_back(poleIndex);
        }
    }

    void addRowStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex)
    {
        for (auto i = 0u; i < tessellation; i++) {
            uint16_t d0 = startIndex + i;
            uint16_t d1 = d0 + 1u;
            uint16_t u0 = d0 + tessellation + 1u;
            uint16_t u1 = d1 + tessellation + 1u;
            indices.emplace_back(d0);
            indices.emplace_back(d1);
            indices.emplace_back(u1);
            indices.emplace_back(u1);
            indices.emplace_back(u0);
            indices.emplace_back(d0);
        }
    }
}

void magma::buildSphereMesh(Mesh& mesh)
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    auto tessellation = 8u;
    auto radius = 1.f;

    std::vector<glm::vec3> positions;
    std::vector<uint16_t> indices;

    // Reserving
    positions.reserve(tessellation * (tessellation - 2u) + 2u);

    // South pole
    addPoleStrip(indices, tessellation, 1u, 0u, true);
    positions.emplace_back(0.f, 0.f, -radius);

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
    const uint16_t indexStart = northPoleIndex - tessellation - 1u;
    addPoleStrip(indices, tessellation, indexStart, northPoleIndex, false);
    positions.emplace_back(glm::vec3{0.f, 0.f, radius});

    // Apply the geometry
    mesh.verticesCount(positions.size());
    mesh.indices(indices);
    mesh.verticesPositions(positions);
    mesh.computeFlatNormals();
    mesh.computeTangents();
}
