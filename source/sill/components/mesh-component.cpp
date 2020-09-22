#include <lava/sill/components/mesh-component.hpp>

#include <lava/chamber/interpolation-tools.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>

using namespace lava;
using namespace lava::chamber;
using namespace lava::sill;

MeshComponent::MeshComponent(Entity& entity, uint8_t sceneIndex)
    : IComponent(entity)
    , IMesh(m_entity.engine().scene(sceneIndex), true)
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_transformComponent.onWorldTransformChanged([this]() { m_nodesDirty = true; });
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
            for (const auto& callback : animationInfo.loopStartCallbacks) {
                callback();
            }
        }

        time += animationInfo.factor * dt;

        const bool reversed = (animationInfo.factor < 0.f);
        const uint32_t stepIncrement = (reversed) ? -1u : 1u;

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

            auto translation = node.translation;
            auto rotation = node.rotation;
            auto scaling = node.scaling;

            for (auto& channelInfo : channelsInfosPair.second) {
                const auto& channel = channelInfo.channel;
                auto& step = channelInfo.step;

                const uint32_t stepEnd = (reversed) ? 0u : channel.timeSteps.size() - 1u;

                // Find the next step of the keyframes, and advance if neccessary
                uint32_t nextStep;
                float nextTime = 0.f;
                while (!channelInfo.paused) {
                    nextStep = step + stepIncrement;
                    nextTime = channel.timeSteps[nextStep];

                    // Advance a step only if we've gone too far.
                    if ((!reversed && time <= nextTime) ||
                        (reversed && time >= nextTime)) {
                            break;
                    }
                    step += stepIncrement;

                    // Pausing if we went too far in the animation.
                    if (step == stepEnd) {
                        channelInfo.paused = true;
                        animationInfo.pausedChannelsCount += 1u;
                    }
                }

                auto previousTime = channel.timeSteps[step];
                if ((!reversed && time >= previousTime) ||
                    (reversed && time <= previousTime)) {
                    auto timeRange = std::max(std::abs(nextTime - previousTime), 0.001f);
                    auto t = std::abs(time - previousTime) / timeRange;

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

            node.parentSpaceMatrix =
                glm::translate(glm::mat4(1.f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.f), scaling);
        }
    }

    if (nodesAnimated) {
        updateNodesEntitySpaceMatrices();
        m_nodesDirty = true;
    }
}

void MeshComponent::updateFrame()
{
    if (m_nodesDirty) {
        updateNodesWorldMatrices();
        m_nodesDirty = false;
    }
}

// ----- Helpers

magma::Mesh& MeshComponent::primitive(uint32_t nodeIndex, uint32_t primitiveIndex)
{
    return m_nodes[nodeIndex].group->primitive(primitiveIndex);
}

magma::MaterialPtr MeshComponent::material(uint32_t nodeIndex, uint32_t primitiveIndex)
{
    return m_nodes[nodeIndex].group->primitive(primitiveIndex).material();
}

float MeshComponent::distanceFrom(const Ray& ray, PickPrecision pickPrecision) const
{
    const auto& bs = boundingSphere();
    float distance = intersectSphere(ray, bs.center, bs.radius);
    if (distance == 0.f) return 0.f;

    if (pickPrecision == PickPrecision::BoundingSphere) return distance;

    // Reset because we're not sure of hitting geometry.
    distance = 0.f;

    for (const auto& node : nodes()) {
        if (node.group == nullptr) continue;

        for (const auto& primitive : node.group->primitives()) {
            if (!primitive->enabled() ||
                (primitive->renderCategory() != RenderCategory::Opaque &&
                 primitive->renderCategory() != RenderCategory::Translucent)) {
                continue;
            }

            // Check against primitive's bounding sphere
            const auto& primitiveBs = primitive->boundingSphere();
            if (intersectSphere(ray, primitiveBs.center, primitiveBs.radius) == 0.f) continue;

            // Check against primitive's triangles
            const auto& transform = primitive->transform(node.instanceIndex);
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

void MeshComponent::startAnimation(const std::string& hrid, uint32_t loops, float factor)
{
    if (m_animationsInfos.find(hrid) == m_animationsInfos.end()) {
        createAnimationInfo(hrid);
    }

    auto& animationInfo = m_animationsInfos.at(hrid);
    animationInfo.loops = loops;
    animationInfo.factor = factor;
    resetAnimationInfo(animationInfo);
}

void MeshComponent::onAnimationLoopStart(const std::string& hrid, AnimationLoopStartCallback callback)
{
    if (m_animationsInfos.find(hrid) == m_animationsInfos.end()) {
        createAnimationInfo(hrid);
    }

    m_animationsInfos.at(hrid).loopStartCallbacks.emplace_back(std::move(callback));
}

// ----- Debug

void MeshComponent::boundingSpheresVisible(bool boundingSpheresVisible)
{
    if (m_boundingSpheresVisible == boundingSpheresVisible) return;
    m_boundingSpheresVisible = boundingSpheresVisible;

    for (auto& node : m_nodes) {
        if (node.group == nullptr) continue;

        for (auto& primitive : node.group->primitives()) {
            primitive->debugBoundingSphere(boundingSpheresVisible);
        }
    }
}

// ----- Internal

void MeshComponent::updateNodesWorldMatrices()
{
    // @note The root nodes have just no parent!
    auto worldMatrix = m_transformComponent.worldTransform().matrix();
    for (auto& node : m_nodes) {
        if (node.group) {
            node.group->transform(worldMatrix * node.entitySpaceMatrix, node.instanceIndex);
        }
    }
}

void MeshComponent::createAnimationInfo(const std::string& hrid)
{
    auto& animation = m_animations[hrid];
    auto& animationInfo = m_animationsInfos[hrid];

    for (auto& animationChannelPair : animation) {
        auto& channelsInfos = animationInfo.channelsInfos[animationChannelPair.first];
        for (auto& channel : animationChannelPair.second) {
            AnimationChannelInfo channelInfo;
            channelInfo.channel = channel;
            channelsInfos.emplace_back(channelInfo);
            animationInfo.channelsCount += 1u;
        }
    }
}

void MeshComponent::resetAnimationInfo(AnimationInfo& animationInfo) const
{
    bool reversed = (animationInfo.factor < 0.f);

    animationInfo.time = 0.f;
    animationInfo.pausedChannelsCount = 0u;
    for (auto& channelsInfosPair : animationInfo.channelsInfos) {
        for (auto& channelInfo : channelsInfosPair.second) {
            channelInfo.paused = false;
            channelInfo.step = (reversed) ? channelInfo.channel.timeSteps.size() - 1u : 0u;

            // @note On reversed animations, we set the starting time to the last possible step.
            if (reversed && animationInfo.time < channelInfo.channel.timeSteps.back()) {
                animationInfo.time = channelInfo.channel.timeSteps.back();
            }
        }
    }
}
