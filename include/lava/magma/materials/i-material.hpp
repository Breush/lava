#pragma once

namespace lava::magma {
    /**
     * Interface for materials.
     */
    class IMaterial {
    public:
        virtual ~IMaterial() = default;

        // Should be implemented.
        // static std::string shaderImplementation();
        // static uint32_t materialId();
        // static void materialId(uint32_t materialId);

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
