#pragma once

#include <lava/chamber/ThreadLocal.hpp>

namespace lava {

    /// \brief Pointer to a thread-local variable
    template <typename T>
    class ThreadLocalPtr : private ThreadLocal {
    public:
        /// \brief Default constructor
        ///
        /// \param value Optional value to initialize the variable
        ThreadLocalPtr(T* value = NULL);

        /// \brief Overload of unary operator *
        ///
        /// Like raw pointers, applying the * operator returns a
        /// reference to the pointed-to object.
        ///
        /// \return Reference to the thread-local variable
        T& operator*() const;

        /// \brief Overload of operator ->
        ///
        /// Similarly to raw pointers, applying the -> operator
        /// returns the pointed-to object.
        ///
        /// \return Pointer to the thread-local variable
        T* operator->() const;

        /// \brief Conversion operator to implicitly convert the
        ///        pointer to its raw pointer type (T*)
        ///
        /// \return Pointer to the actual object
        operator T*() const;

        /// \brief Assignment operator for a raw pointer parameter
        ///
        /// \param value Pointer to assign
        ///
        /// \return Reference to self
        ThreadLocalPtr<T>& operator=(T* value);

        /// \brief Assignment operator for a ThreadLocalPtr parameter
        ///
        /// \param right ThreadLocalPtr to assign
        ///
        /// \return Reference to self
        ThreadLocalPtr<T>& operator=(const ThreadLocalPtr<T>& right);
    };
}

#include <lava/chamber/ThreadLocalPtr.inl>
