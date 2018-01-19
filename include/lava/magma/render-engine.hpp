#pragma once

#include <functional>
#include <lava/core/filesystem.hpp>
#include <lava/core/viewport.hpp>
#include <lava/magma/uniform.hpp>
#include <memory>

namespace lava::magma {
    class ICamera;
    class IRenderScene;
    class IRenderTarget;
}

namespace lava::magma {
    /**
     * An engine that manages everything that need to be drawn.
     */
    class RenderEngine {
    public:
        RenderEngine();
        ~RenderEngine();

        /// Update animations and internal state.
        void update();

        /// Render the current state to all the targets.
        void draw();

        /**
         * Register a material to the engine.
         *
         * All materials have to be registered before adding anything,
         * especially render scenes or render targets.
         */
        // @todo These definitions could be extracted with some shader introspection.
        // Have our own shading "language"?
        uint32_t registerMaterial(const std::string& hrid, const std::string& shaderImplementation,
                                  const UniformDefinitions& uniformDefinitions);
        uint32_t registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath,
                                          const UniformDefinitions& uniformDefinitions);

        /**
         * Add a view of render scene's camera to a render-target.
         * One can show multiple scenes to the same target
         * or the same scene to multiple targets.
         * Everything is possible.
         *
         * @return A unique identifier to the view generated.
         */
        uint32_t addView(ICamera& camera, IRenderTarget& renderTarget, Viewport viewport);

        /**
        * @name Makers
        * Make a new resource and add it to the engine.

        * Arguments will be forwarded to the constructor.
        * Any resource that match an adder (see below) can be made.
        *
        * ```
        * auto& scene = engine.make<RenderScene>(); // Its lifetime is now managed by the engine.
        * ```
        */
        /// @{
        /// Make a new resource directly.
        template <class T, class... Arguments>
        T& make(Arguments&&... arguments);
        /// Make a new resource using a custom maker.
        template <class T, class... Arguments>
        T& make(std::function<void(T&)> maker, Arguments&&... arguments);
        /// @}

        /**
         * @name Adders
         * Add a resource that has already been created.
         *
         * Its ownership goes to the engine.
         * For convenience, you usually want to use makers (see above).
         */
        /// @{
        void add(std::unique_ptr<IRenderScene>&& renderScene);
        void add(std::unique_ptr<IRenderTarget>&& renderTarget);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}

#include <lava/magma/render-engine.inl>
