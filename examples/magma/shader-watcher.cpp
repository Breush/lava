/**
 * Shows how one can enable the shader watcher
 * to be updated on-the-fly.
 */

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app("ashe - magma | Shader watcher");

    // Go and open ./data/tmp/watched-material.shmag in your text editor
    // while this example is running.
    // Any modfication (on save) will be repercuted instantly in the rendering.
    system("cp ./examples/magma/ashe-material.shmag ./data/tmp/watched-material.shmag");

    // Register the material with registerMaterialFromFile() - it will be watched from now on.
    // Use registerMaterial() to not watch a file.
    app.engine().registerMaterialFromFile("watched", "./data/tmp/watched-material.shmag");

    // Making a scene
    auto watchedMaterial = app.scene().makeMaterial("watched");
    watchedMaterial->set("color", {0.4f, 0.05f, 0.6f, 1.f});

    auto& mesh = app.makePlane({1, 1});
    mesh.material(watchedMaterial);

    app.cameraController().origin({2.f, 2.f, 2.f});
    app.cameraController().target({0.f, 0.f, 0.f});

    app.run();

    return EXIT_SUCCESS;
}
