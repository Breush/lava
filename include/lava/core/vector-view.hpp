#pragma once

#include <vector>

namespace lava {
    /**
     * Allows to iterate on raw memory, with stride or such.
     * It is your job to keep the raw memory valid while this is alive.
     *
     *      // This is our data.
     *      const uint8_t* data = ...;
     *      uint32_t dataSize = ...;
     *
     *      // Now we can construct the view and iterate on it like a vector.
     *      VectorView<float> vectorView(data, dataSize, offset, stride);
     *      for (float element : vectorView) {
     *          ...
     *      }
     */
    template <class T>
    class VectorView {
    public:
        VectorView() {}

        /// Simple conversion.
        VectorView(const std::vector<T>& v)
            : m_data(reinterpret_cast<const uint8_t*>(v.data()))
            , m_size(v.size())
        {
        }

        /// The stride is expressed in number of bytes, and can be 0u to use default.
        VectorView(const uint8_t* data, uint32_t size, uint32_t stride = 0u)
            : m_data(data)
            , m_size(size)
            , m_stride(stride)
            , m_effectiveStride(stride != 0u ? stride : sizeof(T))
        {
        }

        const uint8_t* data() const { return m_data; }
        uint32_t size() const { return m_size; }
        uint32_t stride() const { return m_stride; }

        const T& operator[](const uint32_t index) const
        {
            return *reinterpret_cast<const T*>(m_data + m_effectiveStride * index);
        }

        const T* begin() const { return &operator[](0u); }
        const T* end() const { return &operator[](m_size); }

    private:
        const uint8_t* m_data = nullptr;
        uint32_t m_size = 0u;
        uint32_t m_stride = 0u;
        uint32_t m_effectiveStride = sizeof(T);
    };
}
