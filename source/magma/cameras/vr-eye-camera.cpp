#include <lava/magma/cameras/vr-eye-camera.hpp>

#include "../vulkan/cameras/vr-eye-camera-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

$pimpl_class(VrEyeCamera, RenderScene&, scene, Extent2d, extent);

// ICamera
$pimpl_property_v(VrEyeCamera, lava::Extent2d, extent);
$pimpl_method_const(VrEyeCamera, RenderImage, renderImage);
$pimpl_method_const(VrEyeCamera, RenderImage, depthRenderImage);
$pimpl_property_v(VrEyeCamera, PolygonMode, polygonMode);

ICamera::Impl& VrEyeCamera::interfaceImpl()
{
    return *m_impl;
}
