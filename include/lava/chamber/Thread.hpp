#pragma once

#include <cstdlib>

#include <lava/chamber/Export.hpp>
#include <lava/chamber/NonCopyable.hpp>

namespace lava::priv {
    class ThreadImpl;
    struct ThreadFunc;
}

namespace lava {
    /// \brief Utility class to manipulate threads
    class SFML_SYSTEM_API Thread : NonCopyable {
    public:
        /// \brief Construct the thread from a functor with no argument
        ///
        /// This constructor works for function objects, as well
        /// as free functions.
        ///
        /// Use this constructor for this kind of function:
        /// \code
        /// void function();
        ///
        /// // --- or ----
        ///
        /// struct Functor
        /// {
        ///     void operator()();
        /// };
        /// \endcode
        /// Note: this does *not* run the thread, use launch().
        ///
        /// \param function Functor or free function to use as the entry point of the thread
        template <typename F>
        Thread(F function);

        /// \brief Construct the thread from a functor with an argument
        ///
        /// This constructor works for function objects, as well
        /// as free functions.
        /// It is a template, which means that the argument can
        /// have any type (int, std::string, void*, Toto, ...).
        ///
        /// Use this constructor for this kind of function:
        /// \code
        /// void function(int arg);
        ///
        /// // --- or ----
        ///
        /// struct Functor
        /// {
        ///     void operator()(std::string arg);
        /// };
        /// \endcode
        /// Note: this does *not* run the thread, use launch().
        ///
        /// \param function Functor or free function to use as the entry point of the thread
        /// \param argument argument to forward to the function
        template <typename F, typename A>
        Thread(F function, A argument);

        /// \brief Construct the thread from a member function and an object
        ///
        /// This constructor is a template, which means that you can
        /// use it with any class.
        /// Use this constructor for this kind of function:
        /// \code
        /// class MyClass
        /// {
        /// public:
        ///
        ///     void function();
        /// };
        /// \endcode
        /// Note: this does *not* run the thread, use launch().
        ///
        /// \param function Entry point of the thread
        /// \param object Pointer to the object to use
        template <typename C>
        Thread(void (C::*function)(), C* object);

        /// \brief Destructor
        ///
        /// This destructor calls wait(), so that the internal thread
        /// cannot survive after its lava::Thread instance is destroyed.
        ~Thread();

        /// \brief Run the thread
        ///
        /// This function starts the entry point passed to the
        /// thread's constructor, and returns immediately.
        /// After this function returns, the thread's function is
        /// running in parallel to the calling code.
        void launch();

        /// \brief Wait until the thread finishes
        ///
        /// This function will block the execution until the
        /// thread's function ends.
        /// Warning: if the thread function never ends, the calling
        /// thread will block forever.
        /// If this function is called from its owner thread, it
        /// returns without doing anything.
        void wait();

        /// \brief Terminate the thread
        ///
        /// This function immediately stops the thread, without waiting
        /// for its function to finish.
        /// Terminating a thread with this function is not safe,
        /// and can lead to local variables not being destroyed
        /// on some operating systems. You should rather try to make
        /// the thread function terminate by itself.
        void terminate();

    protected:
        friend class priv::ThreadImpl;

        /// \brief Internal entry point of the thread
        ///
        /// This function is called by the thread implementation.
        void run();

    private:
        priv::ThreadImpl* m_impl;       ///< OS-specific implementation of the thread
        priv::ThreadFunc* m_entryPoint; ///< Abstraction of the function to run
    };
}

#include <lava/chamber/Thread.inl>
