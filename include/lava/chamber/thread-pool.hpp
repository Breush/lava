#pragma once

#include <lava/chamber/thread.hpp>

namespace lava::chamber {
    /**
     * On-demand thread pool, meaning that all threads will wait for new jobs
     * to execute and never stop until destructed.
     */
    class ThreadPool {
        static constexpr auto POOL_SIZE = 6u;

    public:
        ThreadPool();

        /// Add a job to be done.
        void job(std::function<void()> job);

        /// Wait for the all jobs and threads to be done.
        void wait();

    protected:
        // Start a job on the specified thread.
        void startJob(Thread& thread, std::function<void()> job);

        // Called automatically whenever a job within a thread finishes.
        void jobFinished(Thread& thread);

    private:
        std::array<Thread, POOL_SIZE> m_threads;
        std::vector<Thread*> m_availableThreads;
        std::vector<std::function<void()>> m_pendingJobs;
        std::mutex m_mutex;
    };
}
