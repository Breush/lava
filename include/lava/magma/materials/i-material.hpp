#pragma once

namespace lava::magma {
    /**
     * Interface for materials.
     */
    class IMaterial {
    public:
        virtual ~IMaterial() = default;

        /// Human-readable identifier.
        // static std::string hrid();

        /// GLSL source code.
        // static std::string shaderImplementation();

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
