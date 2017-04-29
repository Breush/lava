#pragma once

#include <lava/chamber/NonCopyable.hpp>
#include <pthread.h>

namespace lava::priv {

    /// \brief Unix implementation of mutexes

    class MutexImpl : NonCopyable {
    public:
        MutexImpl();
        ~MutexImpl();

        /// \brief Lock the mutex
        void lock();

        /// \brief Unlock the mutex
        void unlock();

    private:
        pthread_mutex_t m_mutex; ///< pthread handle of the mutex
    };
}
