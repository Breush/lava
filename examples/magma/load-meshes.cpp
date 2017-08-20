/**
 * Shows how to load meshes using magma rendering-engine.
 */

#include "../ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app("ashe - magma | RmMaterial");

    app.scene().make<magma::Mesh>("./assets/models/corset.glb");

    app.run();

    return EXIT_SUCCESS;
}
