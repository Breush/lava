#pragma once

#include <cassert> // @fixme Have our own assert
#include <cstdint>
#include <memory>
#include <vector>
#include <cstring> // memset

namespace lava::chamber {
    /**
     * An ever-growing allocator that allocate fixed-size buckets.
     *
     * The buckets' size is determined at the very first allocation.
     * A bucket is destroyed only when there is nothing more in it.
     */
    class BucketAllocator {
        static constexpr const uint32_t INSTANCES_PER_BUCKET = 100u;

    public:
        template <class T, typename... Args>
        inline T* allocateSized(size_t allocationSize, Args&&... args)
        {
            // If we don't know the bucketSize, set it
            if (m_bucketSize == 0u) {
                m_bucketSize = INSTANCES_PER_BUCKET * allocationSize;
                addBucket();
            }

            assert(allocationSize <= m_bucketSize);

            // Make a new bucket if last one is not big enough
            if ((m_bucketSize - m_buckets.back().usedBytes) < allocationSize) {
                addBucket();
            }

            uint8_t* pointer = m_buckets.back().data.get() + m_buckets.back().usedBytes;
            m_buckets.back().usedBytes += allocationSize;

            // Construct in-place the object
            T* tPointer = new (pointer) T(std::forward<Args>(args)...);

            return tPointer;
        }

        template <class T, typename... Args>
        inline T* allocate(Args&&... args)
        {
            return allocateSized<T>(sizeof(T), std::forward<Args>(args)...);
        }

        template <class T>
        void deallocate(T* tPointer)
        {
            // Calling destructor
            tPointer->~T();

            Bucket* tBucket = nullptr;
            for (auto& bucket : m_buckets) {
                if (reinterpret_cast<uint8_t*>(tPointer) >= bucket.data.get() &&
                    reinterpret_cast<uint8_t*>(tPointer) < bucket.data.get() + m_bucketSize) {
                    tBucket = &bucket;
                    break;
                }
            }

            // Deallocating something not allocated by us!
            if (tBucket == nullptr) {
                assert(false);
                return;
            }

            // Clearing memory for debug
            memset(reinterpret_cast<uint8_t*>(tPointer), 0, sizeof(T));

            // @fixme Also move the bucket to last if empty
            tBucket->allocationsCount -= 1u;
        }

    protected:
        void addBucket()
        {
            // @fixme If any, move last bucket to front (because it is full)
            // And do not reallocate if the new back is not full
            // Well... check if that's a good idea first ;)

            m_buckets.emplace_back();
            m_buckets.back().data = std::make_unique<uint8_t[]>(m_bucketSize);
        }

        struct Bucket {
            std::unique_ptr<uint8_t[]> data = nullptr;
            uint32_t usedBytes = 0u;
            uint32_t allocationsCount = 0u;
        };

    private:
        uint32_t m_bucketSize = 0u;
        std::vector<Bucket> m_buckets;
    };
}
