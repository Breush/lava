#include <lava/magma/scene.hpp>

#include <lava/magma/engine.hpp>

#include "./vulkan/engine-impl.hpp"

using namespace lava;

Scene::Scene(Engine& engine)
    : m_engine(engine)
{
}

Scene::Scene(Engine& engine, Window& window)
    : m_engine(engine)
{
    bind(window);
}

Scene::~Scene()
{
}

void Scene::bind(Window& window)
{
    m_window = &window;

    m_engine.impl().swapChain().initSurface(window);
}

void Scene::render()
{
    // Not yet implemented
}
