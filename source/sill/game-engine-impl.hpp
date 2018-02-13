#pragma once

#include <lava/sill/game-engine.hpp>

#include <lava/dike/physics-engine.hpp>
#include <lava/magma/cameras/orbit-camera.hpp>
#include <lava/magma/lights/point-light.hpp>
#include <lava/magma/render-engine.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>
#include <lava/magma/render-targets/render-window.hpp>
#include <lava/sill/game-entity.hpp>
#include <lava/sill/input-manager.hpp>
#include <lava/sill/material.hpp>
#include <lava/sill/texture.hpp>
#include <vector>

#include "./font-manager.hpp"

namespace lava::sill {
    class GameEngine::Impl {
    public:
        Impl(GameEngine& engine);

        GameEngine& base() { return m_base; }
        const GameEngine& base() const { return m_base; }

        // GameEngine
        InputManager& input() { return m_inputManager; }
        void add(std::unique_ptr<GameEntity>&& gameEntity);
        void add(std::unique_ptr<Material>&& material);
        void add(std::unique_ptr<Texture>&& texture);
        void run();

        // Fonts
        // @fixme FontManager should be accessible to none-impl!
        Font& font(const std::string& hrid) { return m_fontManager.font(hrid); }

        // Getters
        magma::RenderEngine& renderEngine() { return *m_renderEngine; }
        const magma::RenderEngine& renderEngine() const { return *m_renderEngine; }
        magma::RenderWindow& renderWindow() { return *m_renderWindow; }
        const magma::RenderWindow& renderWindow() const { return *m_renderWindow; }
        magma::RenderScene& renderScene() { return *m_renderScene; }
        const magma::RenderScene& renderScene() const { return *m_renderScene; }

        dike::PhysicsEngine& physicsEngine() { return *m_physicsEngine; }
        const dike::PhysicsEngine& physicsEngine() const { return *m_physicsEngine; }

    protected:
        void updateInput();
        void updateEntities();
        void registerMaterials();
        void handleEvent(WsEvent& event);

    private:
        GameEngine& m_base;

        // Rendering
        std::unique_ptr<magma::RenderEngine> m_renderEngine = nullptr;
        magma::RenderWindow* m_renderWindow = nullptr;
        magma::RenderScene* m_renderScene = nullptr;
        magma::OrbitCamera* m_camera = nullptr;
        magma::PointLight* m_light = nullptr;

        // Physics
        std::unique_ptr<dike::PhysicsEngine> m_physicsEngine = nullptr;

        // Fonts
        FontManager m_fontManager;

        // Input
        InputManager m_inputManager;

        // Entities
        std::vector<std::unique_ptr<GameEntity>> m_entities;
        std::vector<std::unique_ptr<GameEntity>> m_pendingAddedEntities;
        std::vector<std::unique_ptr<Material>> m_materials;
        std::vector<std::unique_ptr<Texture>> m_textures;
    };
}
