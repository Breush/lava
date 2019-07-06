#pragma once

#include <lava/sill/components/i-component.hpp>

#include <lava/sill/mesh-animation.hpp>
#include <lava/sill/mesh-node.hpp>

namespace lava::sill {
    class MeshComponent final : public IComponent {
    public:
        using AnimationLoopStartCallback = std::function<void()>;

    public:
        MeshComponent(GameEntity& entity);
        ~MeshComponent();

        // IComponent
        static std::string hrid() { return "mesh"; }
        void update(float dt) override final;

        /// A mesh node holds the geometry hierarchy.
        MeshNode& node(uint32_t index);
        std::vector<MeshNode>& nodes();
        const std::vector<MeshNode>& nodes() const;
        void nodes(std::vector<MeshNode>&& nodes);

        // Animations
        void add(const std::string& hrid, const MeshAnimation& animation);
        /// Start an animation. Use -1u for loops to get infinite looping.
        void startAnimation(const std::string& hrid, uint32_t loops = 1u);
        /// Be warned whenever the animation loops or starts.
        void onAnimationLoopStart(const std::string& hrid, AnimationLoopStartCallback callback);

        // Debug
        bool wireframed() const;
        void wireframed(bool wireframed);

        bool boundingSpheresVisible() const;
        void boundingSpheresVisible(bool boundingSpheresVisible);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
