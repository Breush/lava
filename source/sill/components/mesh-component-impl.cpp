#include "./mesh-component-impl.hpp"

#include <lava/chamber/interpolation-tools.hpp>
#include <lava/sill/components/transform-component.hpp>

#include "../mesh-impl.hpp"

using namespace lava;
using namespace lava::chamber;
using namespace lava::sill;

namespace {
    void updateNodeTransforms(MeshNode& node, const glm::mat4& parentTransform)
    {
        node.worldTransform = parentTransform * node.localTransform;

        if (node.mesh) {
            node.mesh->impl().transform(node.worldTransform);
        }

        for (auto child : node.children) {
            updateNodeTransforms(*child, node.worldTransform);
        }
    }
}

MeshComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_transformComponent.onWorldTransformChanged([this]() { onWorldTransformChanged(); });

    // Init correctly on first creation
    onWorldTransformChanged();
}

void MeshComponent::Impl::update(float dt)
{
    if (m_animationsInfos.size() == 0) return;

    for (auto& animationInfo : m_animationsInfos) {
        auto& time = animationInfo.time;
        time += dt;

        // If all channels are paused, we loop over
        // @fixme Animation behavior (looping?) should be user-defined.
        if (animationInfo.pausedChannelsCount == animationInfo.channelsCount) {
            time = 0.f;
            animationInfo.pausedChannelsCount = 0u;
            for (auto& channelsInfosPair : animationInfo.channelsInfos) {
                for (auto& channelInfo : channelsInfosPair.second) {
                    channelInfo.paused = false;
                }
            }
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
                float nextTime;
                while (!channelInfo.paused) {
                    nextStep = step + 1u;
                    nextTime = channel.timeSteps[nextStep];

                    // Advance a step if we've gone too far.
                    if (time <= nextTime) break;
                    step += 1u;

                    // Pausing if we went too far in the animation.
                    if (step >= channel.timeSteps.size() - 1u) {
                        step = 0u;
                        channelInfo.paused = true;
                        animationInfo.pausedChannelsCount += 1u;
                    }
                }

                if (channelInfo.paused) continue;

                auto previousTime = channel.timeSteps[step];
                if (time >= previousTime) {
                    auto timeRange = nextTime - previousTime;
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
    onWorldTransformChanged();
}

// ----- MeshComponent

void MeshComponent::Impl::nodes(std::vector<MeshNode>&& nodes)
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

void MeshComponent::Impl::add(const MeshAnimation& animation)
{
    auto& animationInfo = m_animationsInfos.emplace_back();

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

// ----- Internal

void MeshComponent::Impl::onWorldTransformChanged()
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
