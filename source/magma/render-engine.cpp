#include <lava/magma/render-engine.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/magma/interfaces/material.hpp>
#include <lava/magma/interfaces/render-target.hpp>

#include "./vulkan/render-engine-impl.hpp"

using namespace lava::magma;

$pimpl_class(RenderEngine);

$pimpl_method(RenderEngine, void, update);
$pimpl_method(RenderEngine, void, draw);

void RenderEngine::add(std::unique_ptr<ICamera>&& camera)
{
    m_impl->add(std::move(camera));
}

void RenderEngine::add(std::unique_ptr<IMaterial>&& material)
{
    m_impl->add(std::move(material));
}

void RenderEngine::add(std::unique_ptr<IMesh>&& mesh)
{
    m_impl->add(std::move(mesh));
}

void RenderEngine::add(std::unique_ptr<IPointLight>&& pointLight)
{
    m_impl->add(std::move(pointLight));
}

void RenderEngine::add(std::unique_ptr<IRenderTarget>&& renderTarget)
{
    m_impl->add(std::move(renderTarget));
}
