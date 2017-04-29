#pragma once

#include <lava/chamber/NonCopyable.hpp>
#include <pthread.h>

namespace lava::priv {

    /// \brief Unix implementation of thread-local storage
    class ThreadLocalImpl : NonCopyable {
    public:
        ThreadLocalImpl();
        ~ThreadLocalImpl();

        /// \brief Set the thread-specific value of the variable
        /// \param value Value of the variable for this thread
        void setValue(void* value);

        /// \brief Retrieve the thread-specific value of the variable
        /// \return Value of the variable for this thread
        void* getValue() const;

    private:
        pthread_key_t m_key; ///< Index of our thread-local storage slot
    };
}