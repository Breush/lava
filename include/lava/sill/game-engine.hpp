#pragma once

#include <memory>

namespace lava::dike {
    class PhysicsEngine;
}

namespace lava::sill {
    class GameEntity;
    class InputManager;
    class Material;
    class Texture;
    class Font;
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
        GameEngine();
        ~GameEngine();

        /// Access input manager.
        InputManager& input();

        /// Access physics engine.
        dike::PhysicsEngine& physicsEngine();

        /**
         * @name Fonts
         */
        /// @{
        Font& font(const std::string& hrid);
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
        void add(std::unique_ptr<GameEntity>&& gameEntity);
        void add(std::unique_ptr<Material>&& material);
        void add(std::unique_ptr<Texture>&& texture);
        /// @}

        /// Main loop.
        void run();

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}

#include <lava/sill/game-engine.inl>
