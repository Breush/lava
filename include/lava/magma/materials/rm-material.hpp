#pragma once

#include <lava/magma/interfaces/material.hpp>

#include <cstdint>
#include <vector>

namespace lava::magma {
    /**
     * Roughness-metallic material.
     */
    class RmMaterial final : public IMaterial {
    public:
        RmMaterial(RenderEngine& engine);
        ~RmMaterial();

        void normal(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
