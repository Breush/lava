#include <lava/magma/vr-tools.hpp>

using namespace lava;

bool magma::vrAvailable()
{
    return vr::VR_IsHmdPresent();
}
