#pragma once

#include <lava/sill/components/i-component.hpp>

#include <lava/core/bounding-sphere.hpp>
#include <lava/core/ray.hpp>
#include <lava/core/render-category.hpp>
#include <lava/sill/mesh-animation.hpp>
#include <lava/sill/mesh-node.hpp>
#include <lava/sill/pick-precision.hpp>

namespace lava::sill {
    class TransformComponent;
}

namespace lava::sill {
    class MeshComponent final : public IComponent {
    public:
        using AnimationLoopStartCallback = std::function<void()>;

    public:
        MeshComponent(GameEntity& entity, uint8_t sceneIndex = 0u);

        magma::Scene& scene() { return *m_scene; }

        // IComponent
        static std::string hrid() { return "mesh"; }
        void update(float dt) final;
        void updateFrame() final;

        /**
         * @name Nodes
         *
         * A mesh node holds the geometry hierarchy.
         */
        /// @{
        MeshNode& node(uint32_t index) { return m_nodes[index]; }
        std::vector<MeshNode>& nodes() { return m_nodes; }
        const std::vector<MeshNode>& nodes() const { return m_nodes; }

        // Remove all previously added nodes.
        void removeNodes() { m_nodes.clear(); }

        // @todo :Terminology These are non-uniform scaling transforms,
        // We might want to rename that matrix then.
        void dirtifyNodesTransforms() { m_nodesTranformsDirty = true; }

        /// Emplace back nodes.
        MeshNode& addNode();
        void addNodes(std::vector<MeshNode>&& nodes);
        /// @}

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
        void add(const std::string& hrid, const MeshAnimation& animation);
        /// Start an animation. Use -1u for loops to get infinite looping. Use negative factor to reversed animation.
        void startAnimation(const std::string& hrid, uint32_t loops = 1u, float factor = 1.f);
        /// Be warned whenever the animation loops or starts.
        void onAnimationLoopStart(const std::string& hrid, AnimationLoopStartCallback callback);
        /// @}

        /**
         * @name Attributes
         */
        /// @{
        /// Changes the category of all primitives.
        RenderCategory category() const { return m_category; }
        void category(RenderCategory category);

        /// Changes the enableness of all primitives.
        bool enabled() const { return m_enabled; }
        void enabled(bool enabled);

        /// Bounding sphere around all bounding spheres of primitives.
        BoundingSphere boundingSphere() const;
        /// @}

        /**
         * @name Debug
         */
        /// @{
        /// Shows bounding spheres around each primitives.
        bool boundingSpheresVisible() const { return m_boundingSpheresVisible; }
        void boundingSpheresVisible(bool boundingSpheresVisible);

        /// Path of the file if read from any.
        const std::string& path() const { return m_path; }
        void path(const std::string& path) { m_path = path; }

        void printHierarchy(std::ostream& s) const;
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
        void updateNodesTransforms();
        void resetAnimationInfo(AnimationInfo& animationInfo) const;

    private:
        TransformComponent& m_transformComponent;
        magma::Scene* m_scene = nullptr;

        // Resources
        std::vector<MeshNode> m_nodes;
        std::unordered_map<std::string, AnimationInfo> m_animationsInfos;
        bool m_nodesTranformsDirty = true;

        // Attributes
        RenderCategory m_category = RenderCategory::Opaque;
        bool m_enabled = true;

        // Debug
        bool m_boundingSpheresVisible = false;
        std::string m_path = "";
    };
}
