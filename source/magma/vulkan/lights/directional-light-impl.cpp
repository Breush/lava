#include "./directional-light-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "../render-image-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"

namespace {
    struct ShadowsLightUbo {
        glm::mat4 transform;
    };
}

using namespace lava::magma;

DirectionalLight::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
{
}

DirectionalLight::Impl::~Impl()
{
    if (m_initialized) {
        m_scene.lightDescriptorHolder().freeSet(m_descriptorSet);
    }
}

//----- ILight

RenderImage DirectionalLight::Impl::shadowsRenderImage() const
{
    return m_scene.lightShadowsRenderImage(m_id);
}

//----- ILight::Impl

void DirectionalLight::Impl::init(uint id)
{
    m_id = id;
    m_descriptorSet = m_scene.lightDescriptorHolder().allocateSet();
    m_uboHolder.init(m_descriptorSet, m_scene.lightDescriptorHolder().uniformBufferBindingOffset(), {sizeof(ShadowsLightUbo)});

    m_initialized = true;
    updateBindings();
}

void DirectionalLight::Impl::renderShadows(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                                           uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
}

//----- DirectionalLight

void DirectionalLight::Impl::translation(const glm::vec3& translation)
{
    m_translation = translation;
    updateTransform();
}

void DirectionalLight::Impl::direction(const glm::vec3& direction)
{
    m_direction = glm::normalize(direction);
    updateTransform();
}

//----- Internal

void DirectionalLight::Impl::updateBindings()
{
    if (!m_initialized) return;

    // @todo We should probably hold two UBOs.
    // First, this one, for the shadow map pass.
    // Second, the LightUbo one, for the epiphany pass.
    ShadowsLightUbo ubo;
    ubo.transform = m_shadowsTransform;
    m_uboHolder.copy(0, ubo);
}

void DirectionalLight::Impl::updateTransform()
{
    // @fixme We might need to position the light to scene bounding extremities

    // @note This `projectionTransform[1][1] *= -1` is due to vulkan conventions
    // being different from openGL ones. The Y axis is in the other way.

    auto projectionTransform = glm::ortho(-5.f, 5.f, -5.f, 5.f, 0.01f, 10.f);
    projectionTransform[1][1] *= -1;

    auto viewTransform = glm::lookAt(m_translation, m_translation + m_direction, glm::vec3(0.f, 0.f, 1.f));
    m_shadowsTransform = projectionTransform * viewTransform;

    updateBindings();
}
