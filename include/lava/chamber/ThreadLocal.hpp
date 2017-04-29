#pragma once

#include <cstdlib>

#include <lava/chamber/Export.hpp>
#include <lava/chamber/NonCopyable.hpp>

namespace lava::priv {
    class ThreadLocalImpl;
}

namespace lava {
    /// \brief Defines variables with thread-local storage
    class SFML_SYSTEM_API ThreadLocal : NonCopyable {
    public:
        /// \brief Default constructor
        ///
        /// \param value Optional value to initialize the variable
        ThreadLocal(void* value = NULL);

        /// \brief Destructor
        ~ThreadLocal();

        /// \brief Set the thread-specific value of the variable
        ///
        /// \param value Value of the variable for the current thread
        void setValue(void* value);

        /// \brief Retrieve the thread-specific value of the variable
        ///
        /// \return Value of the variable for the current thread
        void* getValue() const;

    private:
        priv::ThreadLocalImpl* m_impl; ///< Pointer to the OS specific implementation
    };
}