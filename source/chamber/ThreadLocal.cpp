#include "./Unix/ThreadLocalImpl.hpp"

#include <lava/chamber/ThreadLocal.hpp>

using namespace lava;

ThreadLocal::ThreadLocal(void* value)
{
    m_impl = new priv::ThreadLocalImpl;
    setValue(value);
}

ThreadLocal::~ThreadLocal()
{
    delete m_impl;
}

void ThreadLocal::setValue(void* value)
{
    m_impl->setValue(value);
}

void* ThreadLocal::getValue() const
{
    return m_impl->getValue();
}
