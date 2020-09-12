#include <lava/sill/makers/sphere-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>

#include "./makers-common.hpp"

using namespace lava::sill;
using namespace lava::chamber;

std::function<void(MeshComponent&)> makers::sphereMeshMaker(uint32_t tessellation, float diameter, SphereMeshOptions options)
{
    auto radius = diameter / 2;
    return [tessellation, radius, options](MeshComponent& meshComponent) {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

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

        if (options.siding == SphereSiding::In) {
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
        auto meshGroup = std::make_shared<MeshGroup>(meshComponent.scene());
        auto& primitive = meshGroup->addPrimitive();
        primitive.verticesCount(positions.size());
        primitive.verticesPositions(positions);
        primitive.verticesNormals(normals);
        primitive.verticesTangents(tangents);
        primitive.verticesUvs(uvs);
        primitive.indices(indices);

        auto& node = meshComponent.addNode();
        node.meshGroup = std::move(meshGroup);
    };
}
