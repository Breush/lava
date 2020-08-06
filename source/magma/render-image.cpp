#include <lava/magma/render-image.hpp>

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
    if (this == &renderImage) return *this;

    *m_impl = renderImage.impl();
    return *this;
}
