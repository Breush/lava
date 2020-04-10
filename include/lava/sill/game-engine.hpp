#pragma once

#include <lava/sill/pick-precision.hpp>
#include <lava/sill/managers/font-manager.hpp>
#include <lava/sill/managers/input-manager.hpp>
#include <lava/sill/managers/ui-manager.hpp>
#include <lava/sill/managers/vr-manager.hpp>

#include <functional>
#include <lava/core/filesystem.hpp>
#include <lava/core/ray.hpp>
#include <lava/magma/scene.hpp>
#include <memory>

namespace lava::crater {
    class Window;
}

namespace lava::dike {
    class PhysicsEngine;
}

namespace lava::magma {
    class RenderEngine;
    class WindowRenderTarget;
}

namespace lava::sill {
    class GameEntity;
}

namespace lava::sill {
    /**
     * Game engine.
     *
     * Creates a default scene, with one camera.
     * Handles events in a uniform way.
     */
    class GameEngine final {
    public:
        using WindowExtentChangedCallback = std::function<void(Extent2d)>;

    public:
        GameEngine();
        ~GameEngine();

        /// Access physics engine.
        dike::PhysicsEngine& physicsEngine();

        /// Access the render engine.
        magma::RenderEngine& renderEngine();
        magma::Scene& scene();
        magma::Scene& scene2d();
        magma::Camera& camera2d();
        magma::WindowRenderTarget& windowRenderTarget();

        /// Access the windowing system.
        crater::Window& window();

        /// Log FPS at each second.
        bool fpsCounting() const;
        void fpsCounting(bool fpsCounting);

        /**
         * @name Managers
         */
        /// @{
        FontManager& font() { return m_fontManager; }
        InputManager& input() { return m_inputManager; }
        UiManager& ui() { return m_uiManager; }
        VrManager& vr() { return m_vrManager; }
        /// @}

        /**
         * @name Makers
         * Make a new resource managed by the game engine.
         */
        /// @{
        template <class T, class... Arguments>
        T& make(Arguments&&... arguments);
        /// @}

        /**
         * @name Adders
         * Add a resource that has already been created.
         *
         * Its ownership goes to the engine.
         */
        /// @{
        void add(std::unique_ptr<GameEntity>&& entity);

        void remove(const GameEntity& entity);
        /// @}

        /**
         * @name Materials
         */
        /// @{
        void environmentTexture(const fs::Path& imagesPath);
        void registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath);
        /// @}

        /**
         * @name Tools
         */
        /// @{
        std::vector<GameEntity*>& entities() { return m_entities; }
        const std::vector<GameEntity*>& entities() const { return m_entities; }

        /// Find the closest entity that crosses the ray.
        GameEntity* pickEntity(Ray ray, PickPrecision pickPrecision = PickPrecision::Mesh) const;

        /// Moves a ball to the picking point.
        bool debugEntityPicking() const { return m_debugEntityPicking; }
        void debugEntityPicking(bool debugEntityPicking);

        /// Find an entity by its name (will stop after the first one).
        GameEntity* findEntityByName(const std::string& name) const;
        /// @}

        /**
         * @name Callbacks
         */
        /// @{
        uint32_t onWindowExtentChanged(WindowExtentChangedCallback callback);
        void removeOnWindowExtentChanged(uint32_t id);
        /// @}

        /// Main loop.
        void run();

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;

        // Managers
        FontManager m_fontManager{*this};
        InputManager m_inputManager;
        UiManager m_uiManager{*this};
        VrManager m_vrManager{*this};

        std::vector<GameEntity*> m_entities;
        bool m_debugEntityPicking = false;
    };
}

#include <lava/sill/game-engine.inl>
