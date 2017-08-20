#pragma once

#include <lava/magma/interfaces/material.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

#include <cstdint>
#include <vector>

namespace lava::magma {
    /**
     * Roughness-metallic material.
     */
    class RmMaterial final : public IMaterial {
    public:
        RmMaterial(RenderScene& scene);
        ~RmMaterial();

        // IMaterial
        void init() override final;
        UserData render(UserData data) override final;

        /// Roughness factor that will be multiplied to the roughness map value.
        float roughness() const;
        void roughness(float factor);

        /// Metallic factor that will be multiplied to the metallic map value.
        float metallic() const;
        void metallic(float factor);

        /// Normal map, used for bump mapping, vertex tangent data have to be specified.
        void normal(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

        /// Base color at full, front lighting.
        void baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

        /// Occlusion (Red), metallic (Green) and roughness (Blue) map.
        void metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
