#pragma once

#include <functional>
#include <lava/chamber/macros.hpp>
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
 *
 * Usage: $capsule_instance(SurfaceKHR);
 */
#define $capsule_standalone(Class) $capsule_standalone_do(Class)

#define $capsule_standalone_do(Class)                                                                                            \
    class Class {                                                                                                                \
    public:                                                                                                                      \
        using WrappedClass = vk::Class;                                                                                          \
                                                                                                                                 \
    public:                                                                                                                      \
        Class() {}                                                                                                               \
        ~Class() { cleanup(); }                                                                                                  \
                                                                                                                                 \
        $capsule_casts(Class);                                                                                                   \
                                                                                                                                 \
    protected:                                                                                                                   \
        void cleanup()                                                                                                           \
        {                                                                                                                        \
            if (m_object) {                                                                                                      \
                m_object.destroy();                                                                                              \
            }                                                                                                                    \
            m_object = nullptr;                                                                                                  \
        }                                                                                                                        \
                                                                                                                                 \
    private:                                                                                                                     \
        $capsule_attributes();                                                                                                   \
    }

/**
 * Encapsulate destructor behavior for Vulkan types.
 * This provides an RAII lifetime.
 *
 * Usage: $capsule_instance(SurfaceKHR);
 */
#define $capsule_instance(Class) $capsule_instance_do(Class, $cat(destroy, Class))

#define $capsule_instance_do(Class, deleter)                                                                                     \
    class Class {                                                                                                                \
    public:                                                                                                                      \
        using WrappedClass = vk::Class;                                                                                          \
                                                                                                                                 \
    public:                                                                                                                      \
        Class() = delete;                                                                                                        \
        Class(const vk::Instance& instance)                                                                                      \
            : m_instance(instance)                                                                                               \
        {                                                                                                                        \
        }                                                                                                                        \
        ~Class() { cleanup(); }                                                                                                  \
                                                                                                                                 \
        $capsule_casts(Class);                                                                                                   \
                                                                                                                                 \
    protected:                                                                                                                   \
        void cleanup()                                                                                                           \
        {                                                                                                                        \
            if (m_object) {                                                                                                      \
                m_instance.deleter(m_object, nullptr);                                                                           \
            }                                                                                                                    \
            m_object = nullptr;                                                                                                  \
        }                                                                                                                        \
                                                                                                                                 \
    private:                                                                                                                     \
        $capsule_attributes();                                                                                                   \
        const vk::Instance& m_instance;                                                                                          \
    }

/**
 * Usage: $capsule_device(PipelineLayout);
 *        $capsule_device(DeviceMemory, freeMemory); // Using custom delete function name.
 */
#define $capsule_device(...) $capsule_device_n($args_count(__VA_ARGS__), __VA_ARGS__)
#define $capsule_device_n(n, ...) $cat($capsule_device_, n)(__VA_ARGS__)
#define $capsule_device_1(Class) $capsule_device_do(Class, $cat(destroy, Class))
#define $capsule_device_2(Class, deleter) $capsule_device_do(Class, deleter)

#define $capsule_device_do(Class, deleter)                                                                                       \
    class Class {                                                                                                                \
    public:                                                                                                                      \
        using WrappedClass = vk::Class;                                                                                          \
                                                                                                                                 \
    public:                                                                                                                      \
        Class() = delete;                                                                                                        \
        Class(const vk::Device& device)                                                                                          \
            : m_device(device)                                                                                                   \
        {                                                                                                                        \
        }                                                                                                                        \
        ~Class() { cleanup(); }                                                                                                  \
                                                                                                                                 \
        $capsule_casts(Class);                                                                                                   \
                                                                                                                                 \
    protected:                                                                                                                   \
        void cleanup()                                                                                                           \
        {                                                                                                                        \
            if (m_object) {                                                                                                      \
                m_device.deleter(m_object, nullptr);                                                                             \
            }                                                                                                                    \
            m_object = nullptr;                                                                                                  \
        }                                                                                                                        \
                                                                                                                                 \
    private:                                                                                                                     \
        $capsule_attributes();                                                                                                   \
        const vk::Device& m_device;                                                                                              \
    }

#define $capsule_casts(Class)                                                                                                    \
    WrappedClass vk() const { return m_object; }                                                                                 \
    operator WrappedClass&() { return m_object; }                                                                                \
    operator const WrappedClass&() const { return m_object; }                                                                    \
    const WrappedClass* operator&() const { return &m_object; }                                                                  \
                                                                                                                                 \
    WrappedClass* replace()                                                                                                      \
    {                                                                                                                            \
        cleanup();                                                                                                               \
        return &m_object;                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    void operator=(WrappedClass rhs)                                                                                             \
    {                                                                                                                            \
        if (m_object == rhs) return;                                                                                             \
        cleanup();                                                                                                               \
        m_object = rhs;                                                                                                          \
    }                                                                                                                            \
                                                                                                                                 \
    void operator=(const Class& rhs) { *this = static_cast<const WrappedClass&>(rhs); }

#define $capsule_attributes() WrappedClass m_object = nullptr;
}
