#include <lava/sill/components/mesh-component.hpp>

#include <lava/chamber/interpolation-tools.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava;
using namespace lava::chamber;
using namespace lava::sill;

namespace {
    void updateNodeTransforms(MeshNode& node, const glm::mat4& parentTransform)
    {
        node.worldTransform = parentTransform * node.localTransform;

        if (node.meshGroup) {
            node.meshGroup->transform(node.worldTransform);
        }

        for (auto child : node.children) {
            updateNodeTransforms(*child, node.worldTransform);
        }
    }

    void printNodeHierarchy(const MeshNode& node, std::ostream& s, uint32_t tabs)
    {
        for (auto i = 0u; i < tabs; ++i) {
            s << "    ";
        }

        s << "[MeshNode] " << node.name;
        if (node.meshGroup) {
            s << " (mesh with " << node.meshGroup->primitives().size() << " primitives)";
        }
        s << std::endl;

        for (auto child : node.children) {
            printNodeHierarchy(*child, s, tabs + 1u);
        }
    }
}

MeshComponent::MeshComponent(GameEntity& entity)
    : IComponent(entity)
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_transformComponent.onWorldTransformChanged([this]() { onWorldTransformChanged(); });

    // Init correctly on first creation
    onWorldTransformChanged();
}

void MeshComponent::update(float dt)
{
    bool nodesAnimated = false;

    for (auto& animationInfoPair : m_animationsInfos) {
        auto& animationInfo = animationInfoPair.second;
        if (animationInfo.loops == 0u) continue;
        nodesAnimated = true;

        auto& time = animationInfo.time;

        // Call loop start callbacks
        if (time == 0.f) {
            for (auto callback : animationInfo.loopStartCallbacks) {
                callback();
            }
        }

        time += dt;

        // If all channels are paused, we loop over
        if (animationInfo.pausedChannelsCount == animationInfo.channelsCount) {
            if (animationInfo.loops != -1u) animationInfo.loops -= 1u;
            if (animationInfo.loops == 0u) continue;

            resetAnimationInfo(animationInfo);
            continue;
        }

        for (auto& channelsInfosPair : animationInfo.channelsInfos) {
            auto nodeIndex = channelsInfosPair.first;
            auto& node = m_nodes[nodeIndex];

            auto translation = node.translation();
            auto rotation = node.rotation();
            auto scaling = node.scaling();

            for (auto& channelInfo : channelsInfosPair.second) {
                const auto& channel = channelInfo.channel;
                auto& step = channelInfo.step;

                // Find the next step of the keyframes, and advance if neccessary
                uint32_t nextStep;
                float nextTime = 0.f;
                while (!channelInfo.paused) {
                    nextStep = step + 1u;
                    nextTime = channel.timeSteps[nextStep];

                    // Advance a step if we've gone too far.
                    if (time <= nextTime) break;
                    step += 1u;

                    // Pausing if we went too far in the animation.
                    if (step >= channel.timeSteps.size() - 1u) {
                        channelInfo.paused = true;
                        animationInfo.pausedChannelsCount += 1u;
                    }
                }

                auto previousTime = channel.timeSteps[step];
                if (time >= previousTime) {
                    auto timeRange = std::max(nextTime - previousTime, 0.001f);
                    auto t = (time - previousTime) / timeRange;

                    if (channel.path == MeshAnimationPath::Translation) {
                        translation = interpolate(channel.translation, step, t, timeRange, channel.interpolationType);
                    }
                    else if (channel.path == MeshAnimationPath::Rotation) {
                        rotation = interpolate(channel.rotation, step, t, timeRange, channel.interpolationType);
                    }
                    else if (channel.path == MeshAnimationPath::Scaling) {
                        scaling = interpolate(channel.scaling, step, t, timeRange, channel.interpolationType);
                    }
                }
            }

            node.localTransform =
                glm::translate(glm::mat4(1.f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.f), scaling);
        }
    }

    // @todo We could bee more clever to update only the nodes
    // that need to be updated while being sure it's done only once.
    if (nodesAnimated) {
        onWorldTransformChanged();
    }
}

// ----- Nodes

void MeshComponent::nodes(std::vector<MeshNode>&& nodes)
{
    m_nodes = std::move(nodes);

    // Affect parents to each node.
    for (auto& node : m_nodes) {
        node.parent = nullptr;
    }

    for (auto& node : m_nodes) {
        for (auto& child : node.children) {
            child->parent = &node;
        }
    }

    // Update all meshes transform
    onWorldTransformChanged();
}

MeshNode& MeshComponent::addNode()
{
    MeshNode* rootNode = nullptr;
    if (m_nodes.size() > 0u) {
        rootNode = &m_nodes.at(0u);
    }

    m_nodes.emplace_back();

    // @note As m_nodes might have reallocated some buffer,
    // we update all previous pointers if needed.
    if (rootNode != &m_nodes.at(0u)) {
        for (auto i = 0u; i < m_nodes.size() - 1u; ++i) {
            auto& node = m_nodes.at(i);
            if (node.parent) {
                node.parent = &m_nodes.at(node.parent - rootNode);
            }
            for (auto j = 0u; j < node.children.size(); ++j) {
                node.children.at(j) = &m_nodes.at(node.children.at(j) - rootNode);
            }
        }
    }

    return m_nodes.back();
}

// ----- Helpers

magma::Mesh& MeshComponent::primitive(uint32_t nodeIndex, uint32_t primitiveIndex)
{
    return m_nodes[nodeIndex].meshGroup->primitive(primitiveIndex);
}

magma::Material* MeshComponent::material(uint32_t nodeIndex, uint32_t primitiveIndex)
{
    return m_nodes[nodeIndex].meshGroup->primitive(primitiveIndex).material();
}

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

    // @fixme It might be a good idea to first check against the bounding spheres
    // of each primitive.

    distance = 0.f;

    for (const auto& node : nodes()) {
        if (node.meshGroup == nullptr) continue;

        for (const auto& primitive : node.meshGroup->primitives()) {
            if (primitive->category() != RenderCategory::Opaque && primitive->category() != RenderCategory::Translucent) {
                continue;
            }

            const auto& transform = primitive->transform();
            const auto& indices = primitive->indices();
            const auto& vertices = primitive->unlitVertices();
            for (auto i = 0u; i < indices.size(); i += 3) {
                auto p0 = glm::vec3(transform * glm::vec4(vertices[indices[i]].pos, 1.f));
                auto p1 = glm::vec3(transform * glm::vec4(vertices[indices[i + 1]].pos, 1.f));
                auto p2 = glm::vec3(transform * glm::vec4(vertices[indices[i + 2]].pos, 1.f));

                float t = intersectTriangle(ray, p0, p1, p2);
                if (t > 0.f && (distance == 0.f || t < distance)) {
                    distance = t;
                }
            }
        }
    }

    return distance;
}

// ----- Animations

void MeshComponent::add(const std::string& hrid, const MeshAnimation& animation)
{
    auto& animationInfo = m_animationsInfos[hrid];

    for (auto& animationPair : animation) {
        auto& channelsInfos = animationInfo.channelsInfos[animationPair.first];
        for (auto& channel : animation.at(animationPair.first)) {
            AnimationChannelInfo channelInfo;
            channelInfo.channel = channel;
            channelsInfos.emplace_back(channelInfo);
            animationInfo.channelsCount += 1u;
        }
    }
}

void MeshComponent::startAnimation(const std::string& hrid, uint32_t loops)
{
    auto& animationInfo = m_animationsInfos.at(hrid);
    animationInfo.loops = loops;
    resetAnimationInfo(animationInfo);
}

void MeshComponent::onAnimationLoopStart(const std::string& hrid, AnimationLoopStartCallback callback)
{
    m_animationsInfos.at(hrid).loopStartCallbacks.emplace_back(std::move(callback));
}

// ----- Attributes

void MeshComponent::category(RenderCategory category)
{
    if (m_category == category) return;
    m_category = category;

    for (auto& node : m_nodes) {
        if (node.meshGroup == nullptr) continue;

        for (auto& primitive : node.meshGroup->primitives()) {
            primitive->category(category);
        }
    }
}

BoundingSphere MeshComponent::boundingSphere() const
{
    BoundingSphere boundingSphere;

    for (auto& node : m_nodes) {
        if (node.meshGroup == nullptr) continue;

        for (auto& primitive : node.meshGroup->primitives()) {
            boundingSphere = mergeBoundingSpheres(boundingSphere, primitive->boundingSphere());
        }
    }

    return boundingSphere;
}

// ----- Debug

void MeshComponent::boundingSpheresVisible(bool boundingSpheresVisible)
{
    if (m_boundingSpheresVisible == boundingSpheresVisible) return;
    m_boundingSpheresVisible = boundingSpheresVisible;

    for (auto& node : m_nodes) {
        if (node.meshGroup == nullptr) continue;

        for (auto& primitive : node.meshGroup->primitives()) {
            primitive->debugBoundingSphere(boundingSpheresVisible);
        }
    }
}

void MeshComponent::printHierarchy(std::ostream& s) const
{
    s << "[MeshComponent]" << std::endl;

    if (nodes().size() > 0) {
        printNodeHierarchy(nodes()[0], s, 1u);
    }
}

// ----- Internal

void MeshComponent::onWorldTransformChanged()
{
    // @todo We can be more clever than this and dirtify the transforms,
    // waiting for the next update cycle to effectively update.

    auto modelTransform = m_transformComponent.worldTransform();

    // @note The root nodes have just no parent!
    for (auto& node : m_nodes) {
        if (node.parent != nullptr) continue;
        updateNodeTransforms(node, modelTransform);
    }
}

void MeshComponent::resetAnimationInfo(AnimationInfo& animationInfo) const
{
    animationInfo.time = 0.f;
    animationInfo.pausedChannelsCount = 0u;
    for (auto& channelsInfosPair : animationInfo.channelsInfos) {
        for (auto& channelInfo : channelsInfosPair.second) {
            channelInfo.paused = false;
            channelInfo.step = 0u;
        }
    }
}
