#pragma once

#include <lava/chamber/Export.hpp>
#include <lava/chamber/NonCopyable.hpp>

namespace lava {
    namespace priv {
        class MutexImpl;
    }

    /// \brief Blocks concurrent access to shared resources
    ///        from multiple threads
    class Mutex : NonCopyable {
    public:
        Mutex();
        ~Mutex();

        /// \brief Lock the mutex
        ///
        /// If the mutex is already locked in another thread,
        /// this call will block the execution until the mutex
        /// is released.
        void lock();

        /// \brief Unlock the mutex
        void unlock();

    private:
        priv::MutexImpl* m_mutexImpl; ///< OS-specific implementation
    };
}