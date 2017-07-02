/**
 * Shows how the roughness/metallic material works.
 */

#include "../ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app("ashe - magma | RmMaterial");

    auto& sphereMesh = app.engine().make(magma::makers::sphereMeshMaker(32, 0.5));
    sphereMesh.material(app.engine().make<magma::RmMaterial>());

    app.run();

    return EXIT_SUCCESS;
}
