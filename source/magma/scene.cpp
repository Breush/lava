#include <lava/magma/scene.hpp>

#include <lava/magma/engine.hpp>

#include "./vulkan/engine-impl.hpp"

using namespace lava;

Scene::Scene(Engine& engine)
    : m_engine(engine)
{
}

Scene::~Scene()
{
}

void Scene::render()
{
    // Not yet implemented
}
