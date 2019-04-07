/**
 * Shows how to setup a VR environment using magma rendering-engine.
 */

#include <lava/magma.hpp>

using namespace lava;

// @fixme Put that into ashe:: namespace
magma::Mesh& makeCube(magma::RenderScene& scene, float sideLength)
{
    auto& mesh = scene.make<magma::Mesh>();
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
        // Left
        {halfSideLength, halfSideLength, halfSideLength},
        {halfSideLength, halfSideLength, -halfSideLength},
        {-halfSideLength, halfSideLength, -halfSideLength},
        {-halfSideLength, halfSideLength, halfSideLength},
        // Right
        {-halfSideLength, -halfSideLength, halfSideLength},
        {-halfSideLength, -halfSideLength, -halfSideLength},
        {halfSideLength, -halfSideLength, -halfSideLength},
        {halfSideLength, -halfSideLength, halfSideLength},
        // Front
        {halfSideLength, -halfSideLength, halfSideLength},
        {halfSideLength, -halfSideLength, -halfSideLength},
        {halfSideLength, halfSideLength, -halfSideLength},
        {halfSideLength, halfSideLength, halfSideLength},
        // Back
        {-halfSideLength, halfSideLength, halfSideLength},
        {-halfSideLength, halfSideLength, -halfSideLength},
        {-halfSideLength, -halfSideLength, -halfSideLength},
        {-halfSideLength, -halfSideLength, halfSideLength},
    };

    // Normals (flat shading)
    std::vector<glm::vec3> normals = {
        // Bottom
        {0.f, 0.f, -1.f},
        {0.f, 0.f, -1.f},
        {0.f, 0.f, -1.f},
        {0.f, 0.f, -1.f},
        // Top
        {0.f, 0.f, 1.f},
        {0.f, 0.f, 1.f},
        {0.f, 0.f, 1.f},
        {0.f, 0.f, 1.f},
        // Left
        {0.f, 1.f, 0.f},
        {0.f, 1.f, 0.f},
        {0.f, 1.f, 0.f},
        {0.f, 1.f, 0.f},
        // Right
        {0.f, -1.f, 0.f},
        {0.f, -1.f, 0.f},
        {0.f, -1.f, 0.f},
        {0.f, -1.f, 0.f},
        // Front
        {1.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
        // Back
        {-1.f, 0.f, 0.f},
        {-1.f, 0.f, 0.f},
        {-1.f, 0.f, 0.f},
        {-1.f, 0.f, 0.f},
    };

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

    // Apply the geometry
    mesh.verticesCount(positions.size());
    mesh.verticesPositions(positions);
    mesh.verticesNormals(normals);
    mesh.indices(indices);

    return mesh;
}

int main(void)
{
    // Render engine: the global manager.
    magma::RenderEngine engine;

    // The VR device
    auto& vrTarget = engine.make<magma::VrRenderTarget>();

    // @fixme Should have a way to position the center of the world for VR.

    // Render scene: holds what has to be drawn.
    auto& scene = engine.make<magma::RenderScene>();
    scene.rendererType(magma::RendererType::Forward); // Deferred might be too much for VR

    // Tell the VR render target to exist within this scene.
    vrTarget.bindScene(scene);

    // Lights.
    {
        auto& light = scene.make<magma::PointLight>();
        light.translation({0.2, 0.4, 2});
        light.radius(10.f);
    }

    // Some demo mesh
    auto& firstMesh = makeCube(scene, 0.5);
    firstMesh.translate({0, 0, 0.5});

    // @fixme No way to break out while no companion window.
    while (true) {
        // Render the scene.
        engine.update();
        engine.draw();
    }

    return EXIT_SUCCESS;
}
