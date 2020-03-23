#include <lava/sill/makers/tore-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/game-entity.hpp>

#include "./makers-common.hpp"

using namespace lava;

using namespace lava::sill;
using namespace lava::chamber;

std::function<void(MeshComponent&)> makers::toreMeshMaker(uint32_t bigTessellation, float bigDiameter,
                                                          uint32_t smallTessellation, float smallDiameter)
{
    auto bigRadius = bigDiameter / 2.f;
    auto smallRadius = smallDiameter / 2.f;
    return [bigTessellation, bigRadius, smallTessellation, smallRadius](MeshComponent& meshComponent) {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<uint16_t> indices;

        // Reserving
        positions.reserve(bigTessellation * smallTessellation);
        normals.reserve(positions.size());
        indices.reserve(4u * positions.size()); // Each point is part of 2 triangles in each strip, and is part of 2 strips.

        // We just fill one circle of points that will be rotated around the Z axis
        const auto step = math::TWO_PI / smallTessellation;
        const auto cStep = math::cos(step);
        const auto sStep = math::sin(step);
        glm::vec3 center{bigRadius, 0.f, 0.f};
        glm::vec3 point = glm::vec3{smallRadius, 0.f, 0.f};
        for (auto j = 0u; j < smallTessellation; ++j) {
            positions.emplace_back(center + point);
            normals.emplace_back(glm::normalize(point));
            const auto px = point.x;
            point.x = px * cStep - point.z * sStep;
            point.z = px * sStep + point.z * cStep;
        }

        // Now rotating that circle
        for (auto i = 1u; i < bigTessellation; ++i) {
            glm::mat3 rotationMatrix = glm::rotate(glm::mat4(1.f), i * math::TWO_PI / bigTessellation, glm::vec3{0.f, 0.f, 1.f});
            for (auto j = 0u; j < smallTessellation; ++j) {
                positions.emplace_back(rotationMatrix * positions[j]);
                normals.emplace_back(rotationMatrix * normals[j]);
            }
        }

        // Building indices
        for (auto i = 0u; i < bigTessellation; ++i) {
            auto currentCircle = i * smallTessellation;
            auto nextCircle = ((i + 1u) % bigTessellation) * smallTessellation;
            for (auto j = 0u; j < smallTessellation; ++j) {
                auto j1 = (j + 1u) % smallTessellation;
                indices.emplace_back(nextCircle + j1);
                indices.emplace_back(currentCircle + j1);
                indices.emplace_back(currentCircle + j);
                indices.emplace_back(currentCircle + j);
                indices.emplace_back(nextCircle + j);
                indices.emplace_back(nextCircle + j1);
            }
        }

        // Apply the geometry
        auto mesh = std::make_unique<Mesh>(meshComponent.entity().engine());
        auto& primitive = mesh->addPrimitive();
        primitive.verticesCount(positions.size());
        primitive.verticesPositions(positions);
        primitive.verticesNormals(normals);
        // @todo Tangents
        // @todo UVs
        primitive.indices(indices);

        std::vector<MeshNode> nodes(1u);
        nodes[0u].mesh = std::move(mesh);
        meshComponent.nodes(std::move(nodes));
    };
}
