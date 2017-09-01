#include <lava/magma/render-scenes/render-scene.hpp>

#include "../vulkan/render-scenes/render-scene-impl.hpp"

using namespace lava::magma;

$pimpl_class(RenderScene, RenderEngine&, engine);

// IRenderScene
IRenderScene::Impl& RenderScene::interfaceImpl()
{
    return *m_impl;
}

// Adders
void RenderScene::add(std::unique_ptr<ICamera>&& camera)
{
    m_impl->add(std::move(camera));
}

void RenderScene::add(std::unique_ptr<IMaterial>&& material)
{
    m_impl->add(std::move(material));
}

void RenderScene::add(std::unique_ptr<IMesh>&& mesh)
{
    m_impl->add(std::move(mesh));
}

void RenderScene::add(std::unique_ptr<ILight>&& light)
{
    m_impl->add(std::move(light));
}
