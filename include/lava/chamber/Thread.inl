namespace lava {
    namespace priv {
        // Base class for abstract thread functions
        struct ThreadFunc
        {
            virtual ~ThreadFunc() {}
            virtual void run() = 0;
        };

        // Specialization using a functor (including free functions) with no argument
        template <typename T>
        struct ThreadFunctor : ThreadFunc
        {
            ThreadFunctor(T functor) : m_functor(functor) {}
            virtual void run() { m_functor(); }
            T m_functor;
        };

        // Specialization using a functor (including free functions) with one argument
        template <typename F, typename A>
        struct ThreadFunctorWithArg : ThreadFunc
        {
            ThreadFunctorWithArg(F function, A arg) : m_function(function), m_arg(arg) {}
            virtual void run() { m_function(m_arg); }
            F m_function;
            A m_arg;
        };

        // Specialization using a member function
        template <typename C>
        struct ThreadMemberFunc : ThreadFunc
        {
            ThreadMemberFunc(void (C::*function)(), C* object) : m_function(function), m_object(object) {}
            virtual void run() { (m_object->*m_function)(); }
            void (C::*m_function)();
            C* m_object;
        };
    }

    template <typename F>
    Thread::Thread(F functor) : m_impl(NULL), m_entryPoint(new priv::ThreadFunctor<F>(functor))
    {
    }

    template <typename F, typename A>
    Thread::Thread(F function, A argument) : m_impl(NULL), m_entryPoint(new priv::ThreadFunctorWithArg<F, A>(function, argument))
    {
    }

    template <typename C>
    Thread::Thread(void (C::*function)(), C* object) : m_impl(NULL), m_entryPoint(new priv::ThreadMemberFunc<C>(function, object))
    {
    }
}
