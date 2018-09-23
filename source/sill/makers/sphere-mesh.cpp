#include <lava/sill/makers/sphere-mesh.hpp>

#include <lava/chamber/math.hpp>
#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava;

namespace {
    void addCirclePoints(std::vector<glm::vec3>& points, std::vector<glm::vec2>& uvs, const uint32_t tessellation,
                         const float sphereRadius, const float radius, const float height)
    {
        const auto uvStep = 1.f / tessellation;
        const auto step = chamber::math::TWO_PI / tessellation;
        const auto cStep = chamber::math::cos(step);
        const auto sStep = chamber::math::sin(step);

        glm::vec3 point{radius, 0.f, height};
        glm::vec2 uv{0.f, -height / sphereRadius * 0.5f + 0.5f};
        for (auto j = 0u; j < tessellation; ++j) {
            points.emplace_back(point);
            const auto px = point.x;
            point.x = px * cStep - point.y * sStep;
            point.y = px * sStep + point.y * cStep;

            uvs.emplace_back(uv);
            uv.x += uvStep;
        }

        // Last point is the first one with custom uvs
        points.emplace_back(radius, 0.f, height);
        uvs.emplace_back(1.f, uv.y);
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

using namespace lava::sill;
using namespace lava::chamber;

std::function<void(MeshComponent&)> makers::sphereMeshMaker(uint32_t tessellation, float diameter, SphereMeshOptions options)
{
    auto radius = diameter / 2;
    return [tessellation, radius, options](MeshComponent& meshComponent) {
        std::vector<glm::vec3> positions;
        std::vector<uint16_t> indices;

        // @todo options.coordinatesSystem is currenty unused
        // as there is only one choice
        std::vector<glm::vec2> uvs;

        // Reserving
        positions.reserve(tessellation * (tessellation - 2u) + 2u);
        uvs.reserve(positions.size());

        // South pole
        addPoleStrip(indices, tessellation, 1u, 0u, true);
        positions.emplace_back(0.f, 0.f, -radius);
        uvs.emplace_back(0.5f, 1.f);

        // Main strips
        const auto latitudeStep = math::PI / (tessellation - 1u);
        auto latitude = -math::PI_OVER_TWO + latitudeStep;
        for (auto i = 1u; i < tessellation - 2u; ++i) {
            // Adding the strip indices of that circle and the one over it.
            addRowStrip(indices, tessellation, positions.size());

            // This is a circle at a fixed latitude.
            const auto rclat = radius * math::cos(latitude);
            const auto rslat = radius * math::sin(latitude);
            addCirclePoints(positions, uvs, tessellation, radius, rclat, rslat);
            latitude += latitudeStep;
        }

        // Last row of points
        const auto rclat = radius * math::cos(latitude);
        const auto rslat = radius * math::sin(latitude);
        addCirclePoints(positions, uvs, tessellation, radius, rclat, rslat);

        // North pole
        const uint16_t northPoleIndex = positions.size();
        const uint16_t indexStart = northPoleIndex - tessellation - 1u;
        addPoleStrip(indices, tessellation, indexStart, northPoleIndex, false);
        positions.emplace_back(glm::vec3{0.f, 0.f, radius});
        uvs.emplace_back(0.5f, 0.f);

        // @todo Optimize: alloc vertex data buffer directly and bind the whole thing

        // Normals
        std::vector<glm::vec3> normals;
        normals.reserve(positions.size());
        for (const auto& position : positions) {
            normals.emplace_back(position / radius);
        }

        // Tangents
        std::vector<glm::vec4> tangents;
        tangents.reserve(positions.size());
        tangents.emplace_back(1.f, 0.f, 0.f, 1.f);
        for (auto i = 1u; i < positions.size() - 1u; ++i) {
            const auto& position = positions[i];
            tangents.emplace_back(glm::normalize(glm::vec3(-position.y, position.x, 0.f)), 1.f);
        }
        tangents.emplace_back(1.f, 0.f, 0.f, 1.f);

        if (options.siding == SphereSiding::IN) {
            for (auto i = 0u; i < indices.size(); i += 3u) {
                std::swap(indices[i], indices[i + 1]);
            }

            for (auto& normal : normals) {
                normal *= -1.f;
            }

            for (auto& uv : uvs) {
                uv.x *= -1;
            }
        }

        // Apply the geometry
        auto mesh = std::make_unique<Mesh>();
        auto& primitive = mesh->addPrimitive(meshComponent.entity().engine());
        primitive.verticesCount(positions.size());
        primitive.verticesPositions(positions);
        primitive.verticesNormals(normals);
        primitive.verticesTangents(tangents);
        primitive.verticesUvs(uvs);
        primitive.indices(indices);

        std::vector<MeshNode> nodes(1u);
        nodes[0u].mesh = std::move(mesh);
        meshComponent.nodes(std::move(nodes));
    };
}
