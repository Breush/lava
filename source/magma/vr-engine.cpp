#include <lava/magma/vr-engine.hpp>

// @todo Could be #ifdef
#include "./openvr/vr-engine-impl.hpp"

using namespace lava;
using namespace lava::chamber;
using namespace lava::magma;

VrEngine::VrEngine()
{
    // Pre-allocate interesting devices
    m_devicesInfos[VrDeviceType::Head] = DeviceInfo();
    m_devicesInfos[VrDeviceType::RightHand] = DeviceInfo();
    m_devicesInfos[VrDeviceType::LeftHand] = DeviceInfo();

    m_impl = new Impl(*this);
}

VrEngine::~VrEngine()
{
    delete m_impl;
}

void VrEngine::init()
{
    m_enabled = m_impl->init();
}

void VrEngine::update()
{
    if (!m_enabled) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_impl->update();
}

std::optional<VrEvent> VrEngine::pollEvent()
{
    if (!m_enabled) return std::nullopt;

    return m_impl->pollEvent();
}

VrRenderingNeeds VrEngine::renderingNeeds(VrRenderingNeedsInfo info) const
{
    return m_impl->renderingNeeds(info);
}

// ----- Devices

Mesh& VrEngine::deviceMesh(VrDeviceType deviceType, Scene& scene)
{
    // Do not reload if already in cache.
    if (m_devicesMeshes.find(deviceType) == m_devicesMeshes.end()) {
        logger.info("magma.vr-engine") << "Loading VR device mesh for " << deviceType << "." << std::endl;
        logger.log().tab(1);

        m_devicesMeshes[deviceType] = &m_impl->deviceMesh(deviceType, scene);
    }

    return *m_devicesMeshes[deviceType];
}

// ----- Area

void VrEngine::transform(const glm::mat4& transform)
{
    m_transform = transform;
}

// ----- View

Extent2d VrEngine::renderTargetExtent() const
{
    return m_impl->renderTargetExtent();
}

glm::mat4 VrEngine::eyeProjectionTransform(VrEye eye, float nearClip, float farClip) const
{
    return m_impl->eyeProjectionTransform(eye, nearClip, farClip);
}

glm::mat4 VrEngine::eyeViewTransform(VrEye eye) const
{
    // @todo Not sure why we would need inverse here...
    return glm::inverse(m_devicesInfos.at(VrDeviceType::Head).transform * m_impl->eyeToHeadTransform(eye));
}
