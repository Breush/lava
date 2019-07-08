#pragma once

#include <lava/sill/components/mesh-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class TransformComponent;
    class Material;
}

namespace lava::sill {
    class MeshComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);

        // IComponent
        void update(float dt) final;

        // MeshComponent
        MeshNode& node(uint32_t index) { return m_nodes[index]; }
        std::vector<MeshNode>& nodes() { return m_nodes; }
        const std::vector<MeshNode>& nodes() const { return m_nodes; }
        void nodes(std::vector<MeshNode>&& nodes);
        void add(const std::string& hrid, const MeshAnimation& animation);
        void startAnimation(const std::string& hrid, uint32_t loops);
        bool wireframed() const { return m_wireframed; }
        void wireframed(bool wireframed);
        bool depthless() const { return m_depthless; }
        void depthless(bool depthless);
        bool boundingSpheresVisible() const { return m_boundingSpheresVisible; }
        void boundingSpheresVisible(bool boundingSpheresVisible);

        // Callbacks
        void onAnimationLoopStart(const std::string& hrid, AnimationLoopStartCallback callback);

    private:
        // Internal
        void onWorldTransformChanged();

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
            uint32_t loops = 0u; // How many loops there are left to do. (-1u means infinite)
            uint32_t channelsCount = 0u;
            uint32_t pausedChannelsCount = 0u;
            std::vector<AnimationLoopStartCallback> loopStartCallbacks;
            std::unordered_map<uint32_t, std::vector<AnimationChannelInfo>> channelsInfos;
        };

    private:
        // References
        TransformComponent& m_transformComponent;

        // Resources
        std::vector<MeshNode> m_nodes;
        std::unordered_map<std::string, AnimationInfo> m_animationsInfos;

        // Debug
        bool m_wireframed = false;
        bool m_depthless = false;
        bool m_boundingSpheresVisible = false;
    };
}
