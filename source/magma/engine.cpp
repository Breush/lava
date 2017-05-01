#include <lava/magma/engine.hpp>

#include "./vulkan/engine-impl.hpp"

using namespace lava;

Engine::Engine(lava::Window& window)
{
    m_impl = new priv::EngineImpl(window);
}

Engine::~Engine()
{
    delete m_impl;
}
