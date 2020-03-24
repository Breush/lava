#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace lava::chamber {
    /**
     * On-demand thread, meaning that it will wait for a new job function
     * to execute and never stop until destructed.
     */
    class Thread {
    public:
        Thread();
        ~Thread();

        // Set the next job to be done.
        void job(std::function<void()> job);

        // Wait for the current job to be done.
        void wait();

    protected:
        void run();

    private:
        bool m_destroying = false;
        bool m_newJob = false;
        std::thread m_thread;
        std::function<void()> m_job;
        std::mutex m_jobMutex;
        std::condition_variable m_condition;
    };
}
