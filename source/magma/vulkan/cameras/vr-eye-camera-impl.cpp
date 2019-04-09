#include "./vr-eye-camera-impl.hpp"

#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../ubos.hpp"

using namespace lava::magma;
using namespace lava::chamber;

VrEyeCamera::Impl::Impl(RenderScene& scene, Extent2d extent)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
{
    m_fixesTransform = glm::mat4(-1, 0, 0, 0, // X
                                 0, 0, 1, 0,  // Y
                                 0, 1, 0, 0,  // Z
                                 0, 0, 0, 1);

    this->extent(extent);
}

VrEyeCamera::Impl::~Impl()
{
    if (m_initialized) {
        m_scene.cameraDescriptorHolder().freeSet(m_descriptorSet);
    }
}

//----- ICamera

RenderImage VrEyeCamera::Impl::renderImage() const
{
    return m_scene.cameraRenderImage(m_id);
}

RenderImage VrEyeCamera::Impl::depthRenderImage() const
{
    return m_scene.cameraDepthRenderImage(m_id);
}

void VrEyeCamera::Impl::polygonMode(PolygonMode polygonMode)
{
    m_polygonMode = polygonMode;

    if (m_initialized) {
        m_scene.updateCamera(m_id);
    }
}

//----- ICamera::Impl

void VrEyeCamera::Impl::init(uint32_t id)
{
    m_id = id;
    m_descriptorSet = m_scene.cameraDescriptorHolder().allocateSet("orbit-camera." + std::to_string(id));
    m_uboHolder.init(m_descriptorSet, m_scene.cameraDescriptorHolder().uniformBufferBindingOffset(), {sizeof(vulkan::CameraUbo)});

    m_initialized = true;
    updateBindings();
}

void VrEyeCamera::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                               uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
}

void VrEyeCamera::Impl::extent(Extent2d extent)
{
    // Ignoring resize with same extent
    if (m_extent.width == extent.width && m_extent.height == extent.height) {
        return;
    }

    m_extent.width = extent.width;
    m_extent.height = extent.height;

    if (m_initialized) {
        m_scene.updateCamera(m_id);
    }
}

//----- Internal interface

void VrEyeCamera::Impl::update(vr::EVREye eye, const glm::mat4& hmdTransform)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto& vrSystem = m_scene.engine().vrSystem();

    vr::HmdMatrix44_t mat = vrSystem.GetProjectionMatrix(eye, 0.1f, 100.f);
    auto leftProjectionTransform =
        glm::mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0], mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
                  mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
    leftProjectionTransform[1][1] *= -1;
    forceProjectionTransform(leftProjectionTransform);

    auto ethMat = vrSystem.GetEyeToHeadTransform(eye);
    auto leftEyeToHeadTransform =
        glm::mat4(ethMat.m[0][0], ethMat.m[1][0], ethMat.m[2][0], 0.f, ethMat.m[0][1], ethMat.m[1][1], ethMat.m[2][1], 0.f,
                  ethMat.m[0][2], ethMat.m[1][2], ethMat.m[2][2], 0.f, ethMat.m[0][3], ethMat.m[1][3], ethMat.m[2][3], 1.f);

    forceViewTransform(hmdTransform * leftEyeToHeadTransform);
}

void VrEyeCamera::Impl::forceProjectionTransform(const glm::mat4& projectionTransform)
{
    m_projectionTransform = projectionTransform;
}

void VrEyeCamera::Impl::forceViewTransform(glm::mat4 viewTransform)
{
    m_translation = glm::vec3(viewTransform[3]);
    m_viewTransform = glm::inverse(viewTransform);

    updateFrustum();
    updateBindings();
}

void VrEyeCamera::Impl::changeImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    m_scene.changeCameraRenderImageLayout(m_id, imageLayout, commandBuffer);
}

//----- Private

// @todo This is duplicated with OrbitCamera,
// it could exist in ICameraImpl
void VrEyeCamera::Impl::updateFrustum()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto viewTransformInverse = glm::inverse(m_viewTransform * m_fixesTransform);
    auto projectionTransformInverse = glm::inverse(m_projectionTransform);

    auto topLeftLocal = projectionTransformInverse * glm::vec4(-1, -1, 0, 1);
    auto topRightLocal = projectionTransformInverse * glm::vec4(1, -1, 0, 1);
    auto bottomLeftLocal = projectionTransformInverse * glm::vec4(-1, 1, 0, 1);
    auto bottomRightLocal = projectionTransformInverse * glm::vec4(1, 1, 0, 1);

    auto topLeft = glm::vec3(viewTransformInverse * (topLeftLocal / topLeftLocal.w));
    auto topRight = glm::vec3(viewTransformInverse * (topRightLocal / topRightLocal.w));
    auto bottomLeft = glm::vec3(viewTransformInverse * (bottomLeftLocal / bottomLeftLocal.w));
    auto bottomRight = glm::vec3(viewTransformInverse * (bottomRightLocal / bottomRightLocal.w));

    // Left plane
    m_frustum.leftNormal = glm::normalize(glm::cross(topLeft - m_translation, bottomLeft - m_translation));
    m_frustum.leftDistance = glm::dot(m_translation, m_frustum.leftNormal);

    // Right plane
    m_frustum.rightNormal = glm::normalize(glm::cross(bottomRight - m_translation, topRight - m_translation));
    m_frustum.rightDistance = glm::dot(m_translation, m_frustum.rightNormal);

    // Bottom plane
    m_frustum.bottomNormal = glm::normalize(glm::cross(bottomLeft - m_translation, bottomRight - m_translation));
    m_frustum.bottomDistance = glm::dot(m_translation, m_frustum.bottomNormal);

    // Top plane
    m_frustum.topNormal = glm::normalize(glm::cross(topRight - m_translation, topLeft - m_translation));
    m_frustum.topDistance = glm::dot(m_translation, m_frustum.topNormal);

    // Forward
    const auto n = 0.1f; // @note Keep consistent with update() values
    const auto f = 100.f;
    auto forwardNormal = glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft));
    auto cameraDistance = glm::dot(m_translation, forwardNormal);
    m_frustum.forward = forwardNormal;
    m_frustum.near = cameraDistance + n;
    m_frustum.far = cameraDistance + f;
}

void VrEyeCamera::Impl::updateBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    vulkan::CameraUbo ubo;
    ubo.view = m_viewTransform * m_fixesTransform;
    ubo.projection = m_projectionTransform;
    ubo.wPosition = glm::vec4(m_translation, 1.f);
    ubo.extent = glm::uvec2(m_extent.width, m_extent.height);
    m_uboHolder.copy(0, ubo);
}
