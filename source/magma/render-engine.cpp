#include <lava/magma/render-engine.hpp>

#include <lava/chamber/pimpl.hpp>
#include <lava/magma/interfaces/render-target.hpp>
#include <lava/magma/mrr-material.hpp> // @todo Should be an interface

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

void RenderEngine::add(std::unique_ptr<IMaterial>&& material)
{
    m_impl->add(std::move(material));
}

void RenderEngine::add(std::unique_ptr<IMesh>&& mesh)
{
    m_impl->add(std::move(mesh));
}
