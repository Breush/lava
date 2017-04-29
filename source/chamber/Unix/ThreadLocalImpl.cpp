#include "./ThreadLocalImpl.hpp"

using namespace lava::priv;

ThreadLocalImpl::ThreadLocalImpl() : m_key(0)
{
    pthread_key_create(&m_key, NULL);
}

ThreadLocalImpl::~ThreadLocalImpl()
{
    pthread_key_delete(m_key);
}

void ThreadLocalImpl::setValue(void* value)
{
    pthread_setspecific(m_key, value);
}

void* ThreadLocalImpl::getValue() const
{
    return pthread_getspecific(m_key);
}
