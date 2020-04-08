#pragma once

#include <lava/sill/game-engine.hpp>

#include <lava/sill/game-entity.hpp>
#include <lava/sill/input-manager.hpp>

#include "./font-manager.hpp"

namespace lava::sill {
    class GameEngine::Impl {
    public:
        Impl(GameEngine& engine);
        ~Impl();

        /**
         * @name GameEngine
         */
        /// @{
        GameEngine& engine() { return m_engine; }
        const GameEngine& engine() const { return m_engine; }
        InputManager& input() { return m_inputManager; }
        dike::PhysicsEngine& physicsEngine() { return *m_physicsEngine; }
        flow::AudioEngine& audioEngine() { return *m_audioEngine; }
        crater::Window& window() { return *m_window; }

        bool fpsCounting() const { return m_fpsCounting; }
        void fpsCounting(bool fpsCounting) { m_fpsCounting = fpsCounting; }

        // Fonts
        Font& font(const std::string& hrid, uint32_t size) { return m_fontManager.font(hrid, size); }

        // Adders
        void add(std::unique_ptr<GameEntity>&& gameEntity);
        void remove(const GameEntity& gameEntity);

        // Materials
        void environmentTexture(const fs::Path& imagesPath);
        void registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath);

        void run();
        /// @}

        // Getters
        magma::RenderEngine& renderEngine() { return *m_renderEngine; }
        const magma::RenderEngine& renderEngine() const { return *m_renderEngine; }
        magma::WindowRenderTarget& windowRenderTarget() { return *m_windowRenderTarget; }
        const magma::WindowRenderTarget& windowRenderTarget() const { return *m_windowRenderTarget; }
        magma::Scene& scene() { return *m_scene; }
        const magma::Scene& scene() const { return *m_scene; }
        magma::Scene& scene2d() { return *m_scene2d; }
        const magma::Scene& scene2d() const { return *m_scene2d; }
        // @todo Inconsistency: It is CameraComponent that holds the 3D cameras but here it's stored in GameEngine.
        magma::Camera& camera2d() { return *m_camera2d; }
        const magma::Camera& camera2d() const { return *m_camera2d; }

        const std::vector<std::unique_ptr<GameEntity>>& entities() const { return m_entities; }
        std::vector<std::unique_ptr<GameEntity>>& entities() { return m_entities; }

        // Callbacks
        uint32_t onWindowExtentChanged(WindowExtentChangedCallback callback) {
            auto id = m_windowExtentChangedNextId;
            m_windowExtentChangedNextId += 1u;
            m_windowExtentChangedCallbacks[id] = callback;
            return id;
        };
        void removeOnWindowExtentChanged(uint32_t id) {
            if (m_destroying) return;
            m_windowExtentChangedCallbacks.erase(id);
        }

    protected:
        void updateInput();
        void updateEntities(float dt);
        void registerMaterials();
        void handleEvent(WsEvent& event, bool& propagate);

    private:
        GameEngine& m_engine;
        bool m_destroying = false;

        // Rendering
        std::unique_ptr<crater::Window> m_window = nullptr;
        std::unique_ptr<magma::RenderEngine> m_renderEngine = nullptr;
        magma::WindowRenderTarget* m_windowRenderTarget = nullptr;
        magma::VrRenderTarget* m_vrRenderTarget = nullptr;
        magma::Scene* m_scene = nullptr;
        magma::Scene* m_scene2d = nullptr;
        magma::Camera* m_camera2d = nullptr;
        magma::Light* m_light = nullptr;
        magma::DirectionalLightController m_lightController;

        // User control
        bool m_fpsCounting = false;
        std::chrono::nanoseconds m_fpsElapsedTime;
        uint32_t m_fpsCount = 0u;
        magma::Texture* m_environmentTexture = nullptr;

        // Physics
        std::unique_ptr<dike::PhysicsEngine> m_physicsEngine = nullptr;

        // Audio
        std::unique_ptr<flow::AudioEngine> m_audioEngine = nullptr;

        // Fonts
        FontManager m_fontManager;

        // Input
        InputManager m_inputManager;

        // Entities
        std::vector<std::unique_ptr<GameEntity>> m_entities;
        std::vector<std::unique_ptr<GameEntity>> m_pendingAddedEntities;
        std::vector<const GameEntity*> m_pendingRemovedEntities;

        // Callbacks
        std::unordered_map<uint32_t, WindowExtentChangedCallback> m_windowExtentChangedCallbacks; // Key is id
        uint32_t m_windowExtentChangedNextId = 0u;
        float m_windowExtentDelay = -1.f;
        Extent2d m_windowExtent;
    };
}
