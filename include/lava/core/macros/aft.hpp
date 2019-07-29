#pragma once

#include <lava/core/macros.hpp>

namespace lava::chamber::macros {
#define $aft_class(Class, ...)                                                                                                   \
    Class(const Class&) = delete;                                                                                                \
    Class& operator=(const Class&) = delete;                                                                                     \
                                                                                                                                 \
    Class##Aft& aft()                                                                                                            \
    {                                                                                                                            \
        return *reinterpret_cast<Class##Aft*>(reinterpret_cast<uint8_t*>(this) + sizeof(std::aligned_union<0, Class>::type));    \
    }                                                                                                                            \
    const Class##Aft& aft() const                                                                                                \
    {                                                                                                                            \
        return *reinterpret_cast<const Class##Aft*>(reinterpret_cast<const uint8_t*>(this)                                       \
                                                    + sizeof(std::aligned_union<0, Class>::type));                               \
    }
}
