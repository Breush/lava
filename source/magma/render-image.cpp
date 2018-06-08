#include <lava/magma/render-image.hpp>

#include <lava/core/macros.hpp>

#include "./vulkan/render-image-impl.hpp"

using namespace lava::magma;

$pimpl_class(RenderImage);

RenderImage::RenderImage(const RenderImage& renderImage)
    : RenderImage()
{
    *m_impl = renderImage.impl();
}

RenderImage& RenderImage::operator=(const RenderImage& renderImage)
{
    *m_impl = renderImage.impl();
    return *this;
}
