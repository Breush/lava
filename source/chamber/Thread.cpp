#include "./Unix/ThreadImpl.hpp"

#include <lava/chamber/Thread.hpp>

using namespace lava;

Thread::~Thread()
{
    wait();
    delete m_entryPoint;
}

void Thread::launch()
{
    wait();
    m_impl = new priv::ThreadImpl(this);
}

void Thread::wait()
{
    if (m_impl) {
        m_impl->wait();
        delete m_impl;
        m_impl = NULL;
    }
}

void Thread::terminate()
{
    if (m_impl) {
        m_impl->terminate();
        delete m_impl;
        m_impl = NULL;
    }
}

void Thread::run()
{
    m_entryPoint->run();
}
