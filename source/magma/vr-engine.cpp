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

// ----- View

Extent2d VrEngine::renderTargetExtent() const
{
    return m_impl->renderTargetExtent();
}

glm::mat4 VrEngine::eyeProjectionMatrix(VrEye eye, float nearClip, float farClip) const
{
    return m_impl->eyeProjectionMatrix(eye, nearClip, farClip);
}

lava::Transform VrEngine::eyeViewTransform(VrEye eye) const
{
    // @note :CameraViewNeedInverse
    return (m_devicesInfos.at(VrDeviceType::Head).transform * m_impl->eyeToHeadTransform(eye)).inverse();
}
