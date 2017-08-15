#pragma once

namespace lava::magma::vulkan {
    template <class T>
    inline void UboHolder::copy(uint32_t bufferIndex, const T& data)
    {
        copy(bufferIndex, &data, sizeof(T));
    }
}
