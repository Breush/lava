#include <lava/chamber/thread.hpp>

using namespace lava::chamber;

Thread::Thread()
{
    m_thread = std::thread(&Thread::run, this);
}

Thread::~Thread()
{
    if (m_thread.joinable()) {
        wait();
        m_jobMutex.lock();
        m_destroying = true;
        m_condition.notify_one();
        m_jobMutex.unlock();
        m_thread.join();
    }
}

void Thread::job(std::function<void()> job)
{
    std::lock_guard<std::mutex> lock(m_jobMutex);
    m_job = std::move(job);
    m_condition.notify_one();
}

void Thread::wait()
{
    std::unique_lock<std::mutex> lock(m_jobMutex);
    m_condition.wait(lock, [this]() { return m_job == nullptr; });
}

void Thread::run()
{
    while (true) {
        {
            std::unique_lock<std::mutex> lock(m_jobMutex);
            m_condition.wait(lock, [this] { return (m_job != nullptr) || m_destroying; });
            if (m_destroying) {
                break;
            }
        }

        m_job();

        {
            std::lock_guard<std::mutex> lock(m_jobMutex);
            m_job = nullptr;
            m_condition.notify_one();
        }
    }
}
