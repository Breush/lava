#include "./ThreadImpl.hpp"

#include <cassert>
#include <iostream>

#include <lava/chamber/Thread.hpp>

using namespace lava::priv;

ThreadImpl::ThreadImpl(Thread* owner) : m_isActive(true)
{
    m_isActive = pthread_create(&m_thread, NULL, &ThreadImpl::entryPoint, owner) == 0;

    if (!m_isActive) std::cerr << "Failed to create thread" << std::endl;
}

void ThreadImpl::wait()
{
    if (m_isActive) {
        assert(pthread_equal(pthread_self(), m_thread) == 0); // A thread cannot wait for itself!
        pthread_join(m_thread, NULL);
    }
}

void ThreadImpl::terminate()
{
    if (m_isActive) {
        pthread_cancel(m_thread);
    }
}

void* ThreadImpl::entryPoint(void* userData)
{
    // The Thread instance is stored in the user data
    Thread* owner = static_cast<Thread*>(userData);

    // Forward to the owner
    owner->run();

    return NULL;
}
