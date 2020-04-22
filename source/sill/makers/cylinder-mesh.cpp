#include <lava/sill/makers/cylinder-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>

#include "./makers-common.hpp"

using namespace lava;

using namespace lava::sill;
using namespace lava::chamber;

std::function<void(MeshComponent&)> makers::cylinderMeshMaker(uint32_t tessellation, float diameter, float length,
                                                              CylinderMeshOptions options)
{
    auto radius = diameter / 2.f;
    return [tessellation, radius, length, options](MeshComponent& meshComponent) {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        std::vector<glm::vec3> positions;
        std::vector<uint16_t> indices;

        // @todo Currently uvs are only mapped from
        std::vector<glm::vec2> uvs;

        // Reserving
        positions.reserve(tessellation * (tessellation - 2u) + 2u);
        uvs.reserve(positions.size());

        // Adding the strip indices of that circle and the one over it.
        addRowStrip(indices, tessellation, positions.size());

        // This is a circle at a fixed latitude.
        addCirclePoints(positions, uvs, tessellation, length, radius, options.offset - length / 2.f);

        // Last row of points
        addCirclePoints(positions, uvs, tessellation, length, radius, options.offset + length / 2.f);

        // Normals
        std::vector<glm::vec3> normals;
        normals.reserve(positions.size());
        for (const auto& position : positions) {
            normals.emplace_back(glm::normalize(glm::vec3(position.x, position.y, 0.f) / radius));
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

        // Apply baked transform if any
        if (options.transform != glm::mat4(1.f)) {
            for (auto& position : positions) {
                position = glm::vec3(options.transform * glm::vec4(position, 1));
            }

            for (auto& normal : normals) {
                normal = glm::vec3(options.transform * glm::vec4(normal, 0));
            }

            for (auto& tangent : tangents) {
                tangent = glm::vec4(glm::vec3(options.transform * glm::vec4(glm::vec3(tangent), 0)), tangent.w);
            }
        }

        // @note Might refer to wrong normals
        if (options.doubleSided) {
            auto indicesSize = indices.size();
            indices.reserve(2 * indicesSize);
            for (auto i = 0u; i < indicesSize; i += 3) {
                indices.emplace_back(indices[i + 1]);
                indices.emplace_back(indices[i]);
                indices.emplace_back(indices[i + 2]);
            }
        }

        // Apply the geometry
        auto meshGroup = std::make_unique<MeshGroup>(meshComponent.scene());
        auto& primitive = meshGroup->addPrimitive();
        primitive.verticesCount(positions.size());
        primitive.verticesPositions(positions);
        primitive.verticesNormals(normals);
        primitive.verticesTangents(tangents);
        primitive.verticesUvs(uvs);
        primitive.indices(indices);

        std::vector<MeshNode> nodes(1u);
        nodes[0u].meshGroup = std::move(meshGroup);
        meshComponent.nodes(std::move(nodes));
    };
}
