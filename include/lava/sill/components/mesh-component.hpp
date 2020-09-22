#pragma once

#include <lava/sill/components/i-component.hpp>
#include <lava/sill/i-mesh.hpp>

#include <lava/core/ray.hpp>
#include <lava/sill/mesh-node.hpp>
#include <lava/sill/pick-precision.hpp>

namespace lava::sill {
    class TransformComponent;
    class MeshFrameComponent;
}

namespace lava::sill {
    class MeshComponent final : public IComponent, public IMesh {
    public:
        friend class MeshFrameComponent;
        using AnimationLoopStartCallback = std::function<void()>;

    public:
        MeshComponent(Entity& entity, uint8_t sceneIndex = 0u);

        // IComponent
        static std::string hrid() { return "mesh"; }
        void update(float dt) final;
        void updateFrame() final;

        /// The frame component it has been constructed from if any.
        const MeshFrameComponent* frame() const { return m_frame; }

        /**
         * @name Helpers
         */
        /// @{
        magma::Mesh& primitive(uint32_t nodeIndex, uint32_t primitiveIndex);
        magma::MaterialPtr material(uint32_t nodeIndex, uint32_t primitiveIndex);
        float distanceFrom(const Ray& ray, PickPrecision pickPrecision) const;
        /// @}

        /**
         * @name Animations
         */
        /// @{
        /// Start an animation. Use -1u for loops to get infinite looping. Use negative factor to reversed animation.
        void startAnimation(const std::string& hrid, uint32_t loops = 1u, float factor = 1.f);
        /// Be warned whenever the animation loops or starts.
        void onAnimationLoopStart(const std::string& hrid, AnimationLoopStartCallback callback);
        /// @}


        /**
         * @name Debug
         */
        /// @{
        /// Shows bounding spheres around each primitives.
        bool boundingSpheresVisible() const { return m_boundingSpheresVisible; }
        void boundingSpheresVisible(bool boundingSpheresVisible);
        /// @}

    protected:
        struct AnimationChannelInfo {
            MeshAnimationChannel channel;
            // Current step of where we are in the animation.
            // When running, channel.timeSteps[step + 1] > animation.time > channel.timeSteps[step].
            uint32_t step = 0u;
            bool paused = false;
        };

        struct AnimationInfo {
            float time = 0.f;    // Current time of the animation.
            float factor = 1.f;  // Playing speed, if negative, animation is reversed.
            uint32_t loops = 0u; // How many loops there are left to do. (-1u means infinite)
            uint32_t channelsCount = 0u;
            uint32_t pausedChannelsCount = 0u;
            std::vector<AnimationLoopStartCallback> loopStartCallbacks;
            std::unordered_map<uint32_t, std::vector<AnimationChannelInfo>> channelsInfos;
        };

    protected:
        void updateNodesWorldMatrices();
        void createAnimationInfo(const std::string& hrid);
        void resetAnimationInfo(AnimationInfo& animationInfo) const;

    private:
        TransformComponent& m_transformComponent;
        const MeshFrameComponent* m_frame = nullptr;

        // Animations
        std::unordered_map<std::string, AnimationInfo> m_animationsInfos;

        // Debug
        bool m_boundingSpheresVisible = false;
    };
}
