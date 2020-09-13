#pragma once

#include <lava/sill/pick-precision.hpp>
#include <lava/sill/managers/font-manager.hpp>
#include <lava/sill/managers/input-manager.hpp>
#include <lava/sill/managers/ui-manager.hpp>
#include <lava/sill/managers/vr-manager.hpp>

#include <functional>
#include <lava/core/extent.hpp>
#include <lava/core/filesystem.hpp>
#include <lava/core/ray.hpp>
#include <lava/magma/light-controllers/directional-light-controller.hpp>
#include <memory>

namespace lava::crater {
    class Window;
}

namespace lava::dike {
    class PhysicsEngine;
}

namespace lava::flow {
    class AudioEngine;
}

namespace lava::magma {
    class RenderEngine;
    class WindowRenderTarget;
    class VrRenderTarget;
    class Camera;
    class Scene;
}

namespace lava::sill {
    class GameEntity;
    class Prefab;
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

        /// Main loop.
        void run();

        /**
         * @name Sub-system accessors
         */
        /// @{
        crater::Window& window() { return *m_window; }
        magma::RenderEngine& renderEngine() { return *m_renderEngine; }
        dike::PhysicsEngine& physicsEngine() { return *m_physicsEngine; }
        flow::AudioEngine& audioEngine() { return *m_audioEngine; }

        // Windowing
        magma::WindowRenderTarget& windowRenderTarget() { return *m_windowRenderTarget; }
        const magma::WindowRenderTarget& windowRenderTarget() const { return *m_windowRenderTarget; }

        // Rendering
        magma::Scene& scene(uint8_t index = 0u) { return *m_scenes.at(index); }
        magma::Scene& scene2d() { return *m_scene2d; }
        const magma::Scene& scene2d() const { return *m_scene2d; }
        // @todo Inconsistency: It is CameraComponent that holds the 3D cameras but here it's stored in GameEngine.
        magma::Camera& camera2d() { return *m_camera2d; }
        const magma::Camera& camera2d() const { return *m_camera2d; }
        /// Returns the index of the newly allocated scene.
        uint8_t addScene();
        /// @}

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

        void remove(GameEntity& entity);
        /// @}

        /**
         * @name Materials
         */
        /// @{
        void environmentTexture(const fs::Path& imagesPath, uint8_t sceneIndex = 0u);
        void registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath);
        /// @}

        /**
         * @name Callbacks
         */
        /// @{
        uint32_t onWindowExtentChanged(WindowExtentChangedCallback&& callback);
        void removeOnWindowExtentChanged(uint32_t id);
        /// @}

        /**
         * @name Tools
         */
        /// @{
        std::vector<std::unique_ptr<GameEntity>>& entities() { return m_entities; }
        const std::vector<std::unique_ptr<GameEntity>>& entities() const { return m_entities; }

        /// Find the closest entity that crosses the ray.
        GameEntity* pickEntity(const Ray& ray, PickPrecision pickPrecision = PickPrecision::Mesh, float* distance = nullptr) const;

        /// Log FPS at each second.
        bool fpsCounting() const { return m_fpsCounting; }
        void fpsCounting(bool fpsCounting) { m_fpsCounting = fpsCounting; }

        /// Moves a ball to the picking point.
        bool debugEntityPicking() const { return m_debugEntityPicking; }
        void debugEntityPicking(bool debugEntityPicking);

        /// Find an entity by its name (will stop after the first one).
        GameEntity* findEntityByName(const std::string& name) const;
        /// @}

    protected:
        void updateInput();
        void updateEntities(float dt);
        void handleEvent(WsEvent& event, bool& propagate);

    private:
        bool m_destroying = false;

        // Rendering
        std::unique_ptr<crater::Window> m_window;
        std::unique_ptr<magma::RenderEngine> m_renderEngine;
        std::vector<magma::Scene*> m_scenes;
        magma::WindowRenderTarget* m_windowRenderTarget = nullptr;
        magma::VrRenderTarget* m_vrRenderTarget = nullptr;
        magma::Scene* m_scene2d = nullptr;
        magma::Camera* m_camera2d = nullptr;
        magma::Light* m_light = nullptr;
        magma::DirectionalLightController m_lightController;

        // Physics
        std::unique_ptr<dike::PhysicsEngine> m_physicsEngine;

        // Audio
        std::unique_ptr<flow::AudioEngine> m_audioEngine;

        // Entities
        std::vector<GameEntity*> m_allEntities;
        std::vector<std::unique_ptr<GameEntity>> m_entities;
        std::vector<std::unique_ptr<GameEntity>> m_pendingAddedEntities;
        std::vector<const GameEntity*> m_pendingRemovedEntities;

        // Callbacks
        std::unordered_map<uint32_t, WindowExtentChangedCallback> m_windowExtentChangedCallbacks; // Key is id
        uint32_t m_windowExtentChangedNextId = 0u;
        float m_windowExtentDelay = -1.f;
        Extent2d m_windowExtent;

        // Managers
        FontManager m_fontManager{*this};
        InputManager m_inputManager;
        UiManager m_uiManager{*this};
        VrManager m_vrManager{*this};

        // Tools
        bool m_debugEntityPicking = false;
        bool m_fpsCounting = false;
        std::chrono::nanoseconds m_fpsElapsedTime;
        uint32_t m_fpsCount = 0u;
    };
}

#include <lava/sill/game-engine.inl>
