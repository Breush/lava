#include <lava/chamber/Lock.hpp>
#include <lava/chamber/Mutex.hpp>

using namespace lava;

Lock::Lock(Mutex& mutex) : m_mutex(mutex)
{
    m_mutex.lock();
}

Lock::~Lock()
{
    m_mutex.unlock();
}
