#pragma once

#include <cstdint>
#include <vector>

namespace lava {
    // @todo Have this used some time
    class Texture;
}

namespace lava {
    /**
     * Metallic-roughness rendered material.
     */
    class MrrMaterial {
    public:
        MrrMaterial();
        ~MrrMaterial();

        class Impl;
        Impl& impl() { return *m_impl; }

        void baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint32_t channels);

    private:
        Impl* m_impl = nullptr;
    };
}
