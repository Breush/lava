#include <lava/chamber/thread-pool.hpp>

using namespace lava::chamber;

ThreadPool::ThreadPool()
{
    for (auto& thread : m_threads) {
        m_availableThreads.emplace_back(&thread);
    }
}

void ThreadPool::job(std::function<void()> job)
{
    std::scoped_lock lock(m_mutex);

    if (m_availableThreads.empty()) {
        m_pendingJobs.emplace_back(job);
        return;
    }

    auto& thread = *m_availableThreads.back();
    m_availableThreads.pop_back();
    startJob(thread, job);
}

void ThreadPool::wait()
{
    for (auto& thread : m_threads) {
        thread.wait();
    }
}

// ----- Internal

void ThreadPool::startJob(Thread& thread, std::function<void()> job)
{
    thread.job([this, job, &thread] {
        job();
        jobFinished(thread);
    });
}

void ThreadPool::jobFinished(Thread& thread)
{
    std::scoped_lock lock(m_mutex);

    if (!m_pendingJobs.empty()) {
        auto pendingJob = m_pendingJobs.back();
        m_pendingJobs.pop_back();
        startJob(thread, pendingJob);
    } else {
        m_availableThreads.emplace_back(&thread);
    }
}
