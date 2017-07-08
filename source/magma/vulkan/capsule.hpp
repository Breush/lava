#pragma once

#include <functional>
#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {
    /**
     * Encapsulate destructor behavior for Vulkan types.
     * This provides an RAII lifetime.
     */
    template <typename T>
    class Capsule {
    public:
        Capsule()
            : Capsule([](T, VkAllocationCallbacks*) {})
        {
        }

        Capsule(std::function<void(T, VkAllocationCallbacks*)> deletef)
        {
            this->deleter = [=](T obj) { deletef(obj, nullptr); };
        }

        Capsule(const Capsule<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef)
        {
            this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
        }

        Capsule(const Capsule<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef)
        {
            this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
        }

        ~Capsule() { cleanup(); }

        const T* operator&() const { return &object; }

        T* replace()
        {
            cleanup();
            return &object;
        }

        operator T() const { return object; }

        void operator=(T rhs)
        {
            if (rhs != object) {
                cleanup();
                object = rhs;
            }
        }

        template <typename V>
        bool operator==(V rhs)
        {
            return object == T(rhs);
        }

    private:
        T object{VK_NULL_HANDLE};
        std::function<void(T)> deleter;

        void cleanup()
        {
            if (object != VK_NULL_HANDLE) {
                deleter(object);
            }
            object = VK_NULL_HANDLE;
        }
    };

/**
 * Encapsulate destructor behavior for Vulkan types.
 * This provides an RAII lifetime.
 */
#define $capsule_device(Class)                                                                                                   \
    class Class {                                                                                                                \
    public:                                                                                                                      \
        using WrappedClass = vk::Class;                                                                                          \
                                                                                                                                 \
    public:                                                                                                                      \
        Class(const vk::Device& device)                                                                                          \
            : m_device(device)                                                                                                   \
        {                                                                                                                        \
        }                                                                                                                        \
        ~Class() { cleanup(); }                                                                                                  \
                                                                                                                                 \
        $_capsule_casts();                                                                                                       \
                                                                                                                                 \
    protected:                                                                                                                   \
        void cleanup()                                                                                                           \
        {                                                                                                                        \
            if (m_object) {                                                                                                      \
                m_device.destroy##Class(m_object, nullptr);                                                                      \
            }                                                                                                                    \
            m_object = nullptr;                                                                                                  \
        }                                                                                                                        \
                                                                                                                                 \
        $_capsule_attributes();                                                                                                  \
        const vk::Device& m_device;                                                                                              \
    }

#define $_capsule_casts()                                                                                                        \
    operator WrappedClass() const { return m_object; }                                                                           \
    const WrappedClass* operator&() const { return &m_object; }                                                                  \
    WrappedClass* replace()                                                                                                      \
    {                                                                                                                            \
        cleanup();                                                                                                               \
        return &m_object;                                                                                                        \
    }

#define $_capsule_attributes()                                                                                                   \
private:                                                                                                                         \
    WrappedClass m_object = nullptr;
}
