#pragma once

#include <glm/mat4x4.hpp>
#include <lava/core/filesystem.hpp>
#include <lava/core/vr-device-type.hpp>
#include <lava/magma/scene.hpp>
#include <memory>

namespace lava::dike {
    class PhysicsEngine;
}

namespace lava::magma {
    class RenderEngine;
}

namespace lava::sill {
    class GameEntity;
    class InputManager;
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

        /// Access the render engine.
        magma::RenderEngine& renderEngine();
        magma::Scene& scene();

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

        void remove(const GameEntity& gameEntity);
        /// @}

        /**
         * @name Materials
         */
        /// @{
        void environmentTexture(const fs::Path& imagesPath);
        void registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath);
        /// @}

        /**
         * @name VR
         */
        /// @{
        /// Whether a VR system can be used (initialization worked).
        bool vrEnabled() const;

        /// Get whether a device is valid (active and ready to be asked for transform or mesh).
        bool vrDeviceValid(VrDeviceType deviceType) const;

        // @todo Could be nice to access a VrDevice entity instead of transform directly.
        /// Get a device transform.
        const glm::mat4& vrDeviceTransform(VrDeviceType deviceType) const;
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
