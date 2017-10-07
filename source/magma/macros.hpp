#pragma once

#include <cstdint>

#define $magma_material(Class)                                                                                                   \
    namespace {                                                                                                                  \
        uint32_t g_materialId = -1u;                                                                                             \
    }                                                                                                                            \
                                                                                                                                 \
    uint32_t Class::materialId() { return g_materialId; }                                                                        \
    void Class::materialId(uint32_t materialId) { g_materialId = materialId; }
