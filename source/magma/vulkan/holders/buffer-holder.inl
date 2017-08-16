#pragma once

namespace lava::magma::vulkan {
    template <class T>
    inline void BufferHolder::copy(const T& data)
    {
        copy(&data, sizeof(T));
    }
}
