#pragma once

#include <lava/chamber/Export.hpp>
#include <lava/chamber/NonCopyable.hpp>

namespace lava {
    class Mutex;

    /// \brief Automatic wrapper for locking and unlocking mutexes
    class Lock : NonCopyable {
    public:
        /// \brief Construct the lock with a target mutex
        /// The mutex passed to lava::Lock is automatically locked.
        ///
        /// \param mutex Mutex to lock
        explicit Lock(Mutex& mutex);

        ~Lock();

    private:
        Mutex& m_mutex; ///< Mutex to lock / unlock
    };
}
