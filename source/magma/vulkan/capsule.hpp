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
    template <class T>
    class CapsulePP {
    public:
        // @note We are forced to take a raw pointer because of overloaded functions
        // breaking the std::function initializer.
        CapsulePP(const vk::Device& device, void (vk::Device::*deletep)(T, const vk::AllocationCallbacks*) const)
        {
            std::function<void(const vk::Device&, T, const vk::AllocationCallbacks*)> deletef = deletep;
            this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
        }

        ~CapsulePP() { cleanup(); }

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
        T object{nullptr};
        std::function<void(T)> deleter;

        void cleanup()
        {
            if (object) {
                deleter(object);
            }
            object = nullptr;
        }
    };
}
