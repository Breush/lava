#pragma once

#include <lava/magma/materials/i-material.hpp>

#include <string>

namespace lava::magma {
    class RenderScene;
}

namespace lava::magma {
    /**
     * Material used when none is defined.
     */
    class FallbackMaterial final : public IMaterial {
    public:
        FallbackMaterial(RenderScene& scene);
        ~FallbackMaterial();

        // IMaterial
        static std::string hrid();
        static std::string shaderImplementation();
        IMaterial::Impl& interfaceImpl();

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
