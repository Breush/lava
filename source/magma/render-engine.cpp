#include <lava/magma/render-engine.hpp>

#include <lava/chamber/pimpl.hpp>
#include <lava/magma/interfaces/render-target.hpp>

#include "./vulkan/render-engine-impl.hpp"

using namespace lava;

$pimpl_class(RenderEngine);

$pimpl_method(RenderEngine, void, update);
$pimpl_method(RenderEngine, void, draw);

void RenderEngine::add(IRenderTarget& renderTarget)
{
    renderTarget.init(*this);
    m_impl->add(renderTarget);
}
