#pragma once

#include <lava/chamber/NonCopyable.hpp>
#include <lava/config.hpp>

#include <pthread.h>

namespace lava {
    class Thread;
}

namespace lava::priv {

    /// \brief Unix implementation of threads
    class ThreadImpl : NonCopyable {
    public:
        /// \param owner The Thread instance to run
        ThreadImpl(Thread* owner);

        /// \brief Wait until the thread finishes
        void wait();

        /// \brief Terminate the thread
        void terminate();

    private:
        /// \brief Global entry point for all threads
        ///
        /// \param userData User-defined data (contains the Thread instance)
        /// \return Os specific error code
        static void* entryPoint(void* userData);

        pthread_t m_thread; ///< pthread thread instance
        bool m_isActive;    ///< Thread state (active or inactive)
    };
}
