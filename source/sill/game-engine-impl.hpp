#pragma once

#include <lava/sill/game-engine.hpp>

#include <lava/sill/game-entity.hpp>
#include <lava/sill/input-manager.hpp>
#include <lava/sill/material.hpp>
#include <lava/sill/texture.hpp>

#include "./font-manager.hpp"

namespace lava::sill {
    class GameEngine::Impl {
    public:
        using WindowExtentChangedCallback = std::function<void(Extent2d)>;

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

        // Fonts
        Font& font(const std::string& hrid) { return m_fontManager.font(hrid); }

        // Adders
        void add(std::unique_ptr<GameEntity>&& gameEntity);
        void add(std::unique_ptr<Material>&& material);
        void add(std::unique_ptr<Texture>&& texture);
        void remove(const GameEntity& gameEntity);

        // Materials
        void environmentTexture(const fs::Path& imagesPath);
        void registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath);

        // VR
        bool vrEnabled() const { return m_renderEngine->vrEnabled(); }
        bool vrDeviceValid(VrDeviceType deviceType) const { return m_renderEngine->vrDeviceValid(deviceType); }
        const glm::mat4& vrDeviceTransform(VrDeviceType deviceType) const
        {
            return m_renderEngine->vrDeviceTransform(deviceType);
        }

        void run();
        /// @}

        // Getters
        // @fixme One should be able to with the render engine without
        // having to know GameEngine::Impl. For example, if the end user wants
        // to create a new component. We just make an example of that,
        // by having a custom component made inside it.
        magma::RenderEngine& renderEngine() { return *m_renderEngine; }
        const magma::RenderEngine& renderEngine() const { return *m_renderEngine; }
        magma::WindowRenderTarget& windowRenderTarget() { return *m_windowRenderTarget; }
        const magma::WindowRenderTarget& windowRenderTarget() const { return *m_windowRenderTarget; }
        magma::RenderScene& renderScene() { return *m_renderScene; }
        const magma::RenderScene& renderScene() const { return *m_renderScene; }

        // Callbacks
        void onWindowExtentChanged(WindowExtentChangedCallback callback)
        {
            m_windowExtentChangedCallbacks.emplace_back(callback);
        };

    protected:
        void updateInput();
        void updateEntities(float dt);
        void registerMaterials();
        void handleEvent(WsEvent& event);

    private:
        GameEngine& m_engine;

        // Rendering
        std::unique_ptr<crater::Window> m_window = nullptr;
        std::unique_ptr<magma::RenderEngine> m_renderEngine = nullptr;
        magma::WindowRenderTarget* m_windowRenderTarget = nullptr;
        magma::VrRenderTarget* m_vrRenderTarget = nullptr;
        magma::RenderScene* m_renderScene = nullptr;
        magma::DirectionalLight* m_light = nullptr;

        // User control
        bool m_fpsCounterEnabled = false;
        std::chrono::nanoseconds m_fpsElapsedTime;
        uint32_t m_fpsCount = 0u;
        sill::Texture* m_environmentTexture = nullptr;

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
        std::vector<std::unique_ptr<Material>> m_materials;
        std::vector<std::unique_ptr<Texture>> m_textures;

        // Callbacks
        std::vector<WindowExtentChangedCallback> m_windowExtentChangedCallbacks;
    };
}
