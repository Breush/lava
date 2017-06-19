#pragma once

#include <lava/magma/interfaces/material.hpp>

#include <cstdint>
#include <vector>

namespace lava {
    /**
     * Metallic-roughness rendered material.
     */
    class MrrMaterial final : public IMaterial {
    public:
        MrrMaterial();
        ~MrrMaterial();

        void init(RenderEngine& engine) override final;

        class Impl;
        Impl& impl() { return *m_impl; }

        void baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

    private:
        Impl* m_impl = nullptr;
    };
}
