#include <lava/magma/scene.hpp>

#include <lava/magma/engine.hpp>

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
}

void Scene::render()
{
    // Not yet implemented
}
