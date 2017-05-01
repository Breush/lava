#include <lava/magma/engine.hpp>

#include "./vulkan/engine-impl.hpp"

using namespace lava;

Engine::Engine()
{
    m_impl = new priv::EngineImpl();
}

Engine::~Engine()
{
    delete m_impl;
}
