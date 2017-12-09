/**
 * Shows how one can enable the shader watcher
 * to be updated on-the-fly.
 */

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app("ashe - magma | Translucency");

    // Go and open ./data/tmp/watched-material.simpl in your text editor
    // while this example is running.
    // Any modfication (on save) will be repercuted instantly in the rendering.
    system("cp ./examples/magma/ashe-material.simpl ./data/tmp/watched-material.simpl");

    // Register the material with registerMaterialFromFile() - it will be watched from now on.
    // Use registerMaterial() to not watch a file.
    app.engine().registerMaterialFromFile("watched", "./data/tmp/watched-material.simpl",
                                          {{"color", magma::UniformType::VEC4, glm::vec4(1.f)}});

    // Making a scene
    auto& watchedMaterial = app.scene().make<magma::Material>("watched");
    watchedMaterial.set("color", {0.4f, 0.05f, 0.6f, 1.f});

    auto& mesh = app.makePlane({1, 1});
    mesh.material(watchedMaterial);

    app.camera().position({2.f, 2.f, 2.f});
    app.camera().target({0.f, 0.f, 0.f});

    app.run();

    return EXIT_SUCCESS;
}
