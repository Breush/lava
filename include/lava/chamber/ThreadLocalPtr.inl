namespace lava {
    template <typename T>
    ThreadLocalPtr<T>::ThreadLocalPtr(T* value) : ThreadLocal(value)
    {
    }

    template <typename T>
    T& ThreadLocalPtr<T>::operator*() const
    {
        return *static_cast<T*>(getValue());
    }

    template <typename T>
    T* ThreadLocalPtr<T>::operator->() const
    {
        return static_cast<T*>(getValue());
    }

    template <typename T>
    ThreadLocalPtr<T>::operator T*() const
    {
        return static_cast<T*>(getValue());
    }

    template <typename T>
    ThreadLocalPtr<T>& ThreadLocalPtr<T>::operator=(T* value)
    {
        setValue(value);
        return *this;
    }

    template <typename T>
    ThreadLocalPtr<T>& ThreadLocalPtr<T>::operator=(const ThreadLocalPtr<T>& right)
    {
        setValue(right.getValue());
        return *this;
    }
}
