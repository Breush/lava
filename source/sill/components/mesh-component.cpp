#include <lava/sill/components/mesh-component.hpp>

#include "./mesh-component-impl.hpp"

using namespace lava;
using namespace lava::sill;

$pimpl_class_base(MeshComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(MeshComponent, void, update, float, dt);

// MeshComponent
$pimpl_method(MeshComponent, MeshNode&, node, uint32_t, index);
$pimpl_method(MeshComponent, std::vector<MeshNode>&, nodes);
$pimpl_method_const(MeshComponent, const std::vector<MeshNode>&, nodes);

void MeshComponent::nodes(std::vector<MeshNode>&& nodes)
{
    m_impl->nodes(std::move(nodes));
}

$pimpl_method(MeshComponent, void, add, const std::string&, hrid, const MeshAnimation&, animation);
$pimpl_method(MeshComponent, void, startAnimation, const std::string&, hrid, uint32_t, loops);
$pimpl_method(MeshComponent, void, onAnimationLoopStart, const std::string&, hrid, AnimationLoopStartCallback, callback);

$pimpl_method(MeshComponent, void, category, RenderCategory, category);
$pimpl_method_const(MeshComponent, BoundingSphere, boundingSphere);
$pimpl_property_v(MeshComponent, bool, boundingSpheresVisible);

float MeshComponent::distanceFrom(Ray ray, PickPrecision pickPrecision) const
{
    // @fixme Currently not handling PickPrecision::BoundingBox

    const auto& bs = boundingSphere();
    auto rayOriginToBsCenter = bs.center - ray.origin;
    auto distance = glm::dot(ray.direction, rayOriginToBsCenter);
    if (distance <= 0.f) return 0.f;

    auto bsCenterProjection = ray.origin + ray.direction * distance;
    if (glm::length(bs.center - bsCenterProjection) > bs.radius) return 0.f;

    if (pickPrecision != PickPrecision::Mesh) return distance;

    // @fixme It might be a good idea to check against the bounding spheres
    // of each primitive and not entity-wide.

    distance = 0.f;

    for (const auto& node : nodes()) {
        if (node.mesh == nullptr) continue;

        // @note For the ray-triangle intersection test,
        // we're using Real-Time Rendering Fourth Edition - page 965,
        // which projects the ray into the barycentric coordinates system
        // of the triangle to test;
        for (const auto& primitive : node.mesh->primitives()) {
            if (primitive->category() != RenderCategory::Opaque && primitive->category() != RenderCategory::Translucent) {
                continue;
            }

            auto transform = primitive->transform();
            const auto& indices = primitive->indices();
            const auto& vertices = primitive->unlitVertices();
            for (auto i = 0u; i < indices.size(); i += 3) {
                auto p0 = glm::vec3(transform * glm::vec4(vertices[indices[i]].pos, 1.f));
                auto p1 = glm::vec3(transform * glm::vec4(vertices[indices[i + 1]].pos, 1.f));
                auto p2 = glm::vec3(transform * glm::vec4(vertices[indices[i + 2]].pos, 1.f));

                auto e10 = p1 - p0;
                auto e20 = p2 - p0;

                // Ignore triangles with opposite normals
                auto c = glm::cross(e10, e20);
                if (glm::dot(c, ray.origin) < 0.f) continue;

                // Ray might be parallel to triangle
                auto q = glm::cross(ray.direction, e20);
                auto a = glm::dot(e10, q);
                if (a > 0.0001f && a < 0.0001f) continue;

                auto f = 1.f / a;
                auto s = ray.origin - p0;
                auto u = f * glm::dot(s, q);
                if (u < 0.f) continue;

                auto r = glm::cross(s, e10);
                auto v = f * glm::dot(ray.direction, r);
                if (v < 0.f || u + v > 1.f) continue;

                auto t = f * glm::dot(e20, r);
                if (t < 0.f) continue;

                if (distance == 0.f || t < distance) {
                    distance = t;
                }
            }
        }
    }

    return distance;
}
