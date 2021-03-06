#include <lava/sill/makers/plane-mesh.hpp>

#include <lava/sill/i-mesh.hpp>

using namespace lava::sill;

std::function<uint32_t(IMesh&)> makers::planeMeshMaker(float sidesLength, PlaneMeshOptions options)
{
    return makers::planeMeshMaker({sidesLength, sidesLength}, options);
}

std::function<uint32_t(IMesh&)> makers::planeMeshMaker(const glm::vec2& dimensions, PlaneMeshOptions options)
{
    return [=](IMesh& iMesh) -> uint32_t {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        uint32_t verticesCount = options.tessellation.x * options.tessellation.y;
        std::vector<glm::vec3> positions(verticesCount);
        std::vector<glm::vec2> uvs(verticesCount);
        std::vector<glm::vec3> normals(verticesCount, {0.f, 0.f, 1.f});
        std::vector<glm::vec4> tangents(verticesCount, {1.f, 0.f, 0.f, 1.f});
        std::vector<uint16_t> indices;

        const auto halfWidth = dimensions.x / 2.f;
        const auto halfHeight = dimensions.y / 2.f;

        for (auto x = 0u; x < options.tessellation.x; ++x) {
            for (auto y = 0u; y < options.tessellation.y; ++y) {
                auto index = x * options.tessellation.y + y;

                uvs[index].x = float(x) / float(options.tessellation.x - 1u);
                uvs[index].y = float(y) / float(options.tessellation.y - 1u);

                positions[index].x = -halfWidth + uvs[index].x * dimensions.x;
                positions[index].y = -halfHeight + uvs[index].y * dimensions.y;
                positions[index].z = 0.f;
            }
        }

        indices.reserve(6 * (options.tessellation.x - 1u) * (options.tessellation.y - 1u));
        for (auto x = 0u; x < options.tessellation.x - 1u; ++x) {
            for (auto y = 0u; y < options.tessellation.y - 1u; ++y) {
                auto index = x * options.tessellation.y + y;
                indices.emplace_back(index);
                indices.emplace_back(index + options.tessellation.y);
                indices.emplace_back(index + options.tessellation.y + 1u);
                indices.emplace_back(index + options.tessellation.y + 1u);
                indices.emplace_back(index + 1u);
                indices.emplace_back(index);
            }
        }

        // @todo It is double sided, but these vertices refer to wrong normals.
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
        magma::Mesh* primitive;
        auto rootNodeIndex = iMesh.addNode();
        if (options.rootNodeHasGeometry) {
            auto& group = iMesh.nodeMakeGroup(rootNodeIndex);
            primitive = &group.addPrimitive();
        }
        else {
            auto nodeIndex = iMesh.addNode();
            auto& group = iMesh.nodeMakeGroup(nodeIndex);
            primitive = &group.addPrimitive();

            iMesh.nodeAddAbsoluteChild(rootNodeIndex, nodeIndex);
        }

        primitive->verticesCount(positions.size());
        primitive->verticesPositions(positions);
        primitive->verticesNormals(normals);
        primitive->verticesTangents(tangents);
        primitive->verticesUvs(uvs);
        primitive->indices(indices);

        return rootNodeIndex;
    };
}
