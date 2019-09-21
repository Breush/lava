#pragma once

#include <lava/sill/components/i-component.hpp>

#include <lava/core/bounding-sphere.hpp>
#include <lava/core/ray.hpp>
#include <lava/core/render-category.hpp>
#include <lava/sill/mesh-animation.hpp>
#include <lava/sill/mesh-node.hpp>
#include <lava/sill/pick-precision.hpp>

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

        // Helper to access a primitive directly.
        magma::Mesh& primitive(uint32_t nodeIndex, uint32_t primitiveIndex)
        {
            return node(nodeIndex).mesh->primitive(primitiveIndex);
        }

        // Helper to access a material directly.
        magma::Material* material(uint32_t nodeIndex, uint32_t primitiveIndex)
        {
            return node(nodeIndex).mesh->primitive(primitiveIndex).material();
        }

        // Animations
        void add(const std::string& hrid, const MeshAnimation& animation);
        /// Start an animation. Use -1u for loops to get infinite looping.
        void startAnimation(const std::string& hrid, uint32_t loops = 1u);
        /// Be warned whenever the animation loops or starts.
        void onAnimationLoopStart(const std::string& hrid, AnimationLoopStartCallback callback);

        void category(RenderCategory category);

        BoundingSphere boundingSphere() const;
        float distanceFrom(Ray ray, PickPrecision pickPrecision) const;

        // Debug
        bool boundingSpheresVisible() const;
        void boundingSpheresVisible(bool boundingSpheresVisible);
        void printHierarchy(std::ostream& s) const;

        /// Path of the file if read from any.
        const std::string& path() const { return m_path; }
        void path(const std::string& path) { m_path = path; }

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;

        std::string m_path = "";
    };
}
