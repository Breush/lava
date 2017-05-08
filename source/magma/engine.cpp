#include <lava/magma/engine.hpp>

#include "./vulkan/engine-impl.hpp"

using namespace lava;

Engine::Engine(lava::Window& window)
{
    m_impl = new Impl(window);
}

Engine::~Engine()
{
    delete m_impl;
}

void Engine::update()
{
    m_impl->update();
}

void Engine::draw()
{
    m_impl->draw();
}

void Engine::mode(const lava::VideoMode& mode)
{
    m_impl->mode(mode);
}
