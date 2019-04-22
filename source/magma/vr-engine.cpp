#include "./vr-engine.hpp"

#include <lava/magma/material.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>
#include <lava/magma/texture.hpp>
#include <lava/magma/vr-tools.hpp>

using namespace lava;

namespace {
    VrDeviceType deviceTypeHandFromOpenVrDeviceIndex(vr::IVRSystem& vrSystem, uint32_t deviceIndex)
    {
        auto controllerRole = vrSystem.GetControllerRoleForTrackedDeviceIndex(deviceIndex);
        if (controllerRole == vr::TrackedControllerRole_RightHand) {
            return VrDeviceType::RightHand;
        }
        else if (controllerRole == vr::TrackedControllerRole_LeftHand) {
            return VrDeviceType::LeftHand;
        }
        return VrDeviceType::UnknownHand;
    }

    VrButton buttonFromOpenVrButtonId(uint32_t buttonId)
    {
        switch (buttonId) {
        case vr::k_EButton_System: return VrButton::System;
        case vr::k_EButton_ApplicationMenu: return VrButton::Menu;
        case vr::k_EButton_SteamVR_Trigger: return VrButton::Trigger;
        case vr::k_EButton_SteamVR_Touchpad: return VrButton::Touchpad;
        default: return VrButton::Unknown;
        }
    }
}

using namespace lava::chamber;
using namespace lava::magma;

VrEngine::VrEngine()
    : m_devicesPoses(vr::k_unMaxTrackedDeviceCount)
    , m_devicesInfos(vr::k_unMaxTrackedDeviceCount)
{
    // Pre-allocate interesting devices
    m_devicesInfos[VrDeviceType::Head] = DeviceInfo();
    m_devicesInfos[VrDeviceType::RightHand] = DeviceInfo();
    m_devicesInfos[VrDeviceType::LeftHand] = DeviceInfo();
}

VrEngine::~VrEngine() {}

void VrEngine::init()
{
    if (!vrAvailable()) {
        logger.log() << "VR is not available." << std::endl;
        return;
    }

    // Initializing VR system
    vr::EVRInitError error = vr::VRInitError_None;
    m_vrSystem = vr::VR_Init(&error, vr::EVRApplicationType::VRApplication_Scene);

    if (error != vr::VRInitError_None) {
        logger.warning("magma.vr-engine") << "VR seems available but we failed to init VR. Is SteamVR ready?" << std::endl;
    }
    else {
        logger.log() << "VR enabled." << std::endl;
        m_enabled = true;
    }
}

void VrEngine::update()
{
    if (!m_enabled) return;

    // Update all known devices...
    auto error = vr::VRCompositor()->WaitGetPoses(m_devicesPoses.data(), vr::k_unMaxTrackedDeviceCount, nullptr, 0);
    if (error != 0) return;

    for (auto deviceIndex = 0u; deviceIndex < vr::k_unMaxTrackedDeviceCount; ++deviceIndex) {
        const auto& pose = m_devicesPoses[deviceIndex];

        DeviceInfo deviceInfo;
        deviceInfo.valid = pose.bPoseIsValid;
        if (!deviceInfo.valid) continue;

        auto m = pose.mDeviceToAbsoluteTracking.m;
        deviceInfo.transform = glm::mat4(m[0][0], m[1][0], m[2][0], 0.f, // X
                                         m[0][1], m[1][1], m[2][1], 0.f, // Y
                                         m[0][2], m[1][2], m[2][2], 0.f, // Z
                                         m[0][3], m[1][3], m[2][3], 1.f);
        deviceInfo.fixedTransform = m_fixesTransform * deviceInfo.transform;

        deviceInfo.type = m_vrSystem->GetTrackedDeviceClass(deviceIndex);
        deviceInfo.index = deviceIndex;

        if (deviceInfo.type == vr::TrackedDeviceClass_HMD) {
            m_devicesInfos[VrDeviceType::Head] = deviceInfo;
        }
        else if (deviceInfo.type == vr::TrackedDeviceClass_Controller) {
            auto deviceType = deviceTypeHandFromOpenVrDeviceIndex(*m_vrSystem, deviceIndex);
            m_devicesInfos[deviceType] = deviceInfo;
        }
        else if (deviceInfo.type == vr::TrackedDeviceClass_TrackingReference) {
            // @todo Store the info too?
        }
        else {
            logger.warning("magma.vulkan.vr-render-target") << "Unknown VR device type." << std::endl;
        }
    }
}

Extent2d VrEngine::renderTargetExtent()
{
    Extent2d extent;
    m_vrSystem->GetRecommendedRenderTargetSize(&extent.width, &extent.height);
    return extent;
}

glm::mat4 VrEngine::eyeProjectionTransform(Eye eye)
{
    vr::HmdMatrix44_t mat = m_vrSystem->GetProjectionMatrix((eye == Eye::Left) ? vr::Eye_Left : vr::Eye_Right, 0.1f, 100.f);
    auto transform =
        glm::mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0], mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
                  mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
    transform[1][1] *= -1;
    return transform;
}

glm::mat4 VrEngine::eyeToHeadTransform(Eye eye)
{
    auto mat = m_vrSystem->GetEyeToHeadTransform((eye == Eye::Left) ? vr::Eye_Left : vr::Eye_Right);
    auto transform = glm::mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.f, mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.f,
                               mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.f, mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.f);
    return transform;
}

glm::mat4 VrEngine::eyeViewTransform(Eye eye)
{
    return m_devicesInfos[VrDeviceType::Head].transform * eyeToHeadTransform(eye);
}

Mesh& VrEngine::deviceMesh(VrDeviceType deviceType, RenderScene& scene)
{
    // Check if already in cache
    if (m_devicesMeshes[deviceType] != nullptr) {
        return *m_devicesMeshes[deviceType];
    }

    logger.info("magma.vulkan.vr-render-target") << "Loading VR device mesh for " << deviceType << "." << std::endl;
    logger.log().tab(1);

    // Get device name
    auto deviceIndex = m_devicesInfos[deviceType].index;
    uint32_t bufferLength =
        vr::VRSystem()->GetStringTrackedDeviceProperty(deviceIndex, vr::Prop_RenderModelName_String, nullptr, 0, nullptr);
    char* renderModelNameBuffer = new char[bufferLength];
    vr::VRSystem()->GetStringTrackedDeviceProperty(deviceIndex, vr::Prop_RenderModelName_String, renderModelNameBuffer,
                                                   bufferLength, nullptr);
    std::string renderModelName(renderModelNameBuffer);
    delete[] renderModelNameBuffer;

    // Get render model from this name
    vr::RenderModel_t* model;
    vr::EVRRenderModelError error;
    while (1) {
        error = vr::VRRenderModels()->LoadRenderModel_Async(renderModelName.c_str(), &model);
        if (error != vr::VRRenderModelError_Loading) break;
        usleep(100); // @note Ugly, but no choice...
    }

    if (error != vr::VRRenderModelError_None) {
        logger.error("magma.vulkan.vr-render-target") << "Unable to load VR device render model. " << error << std::endl;
    }

    // Get associated diffuse texture
    vr::RenderModel_TextureMap_t* texture;
    while (1) {
        error = vr::VRRenderModels()->LoadTexture_Async(model->diffuseTextureId, &texture);
        if (error != vr::VRRenderModelError_Loading) break;
        usleep(100); // @note Ugly, but no choice...
    }

    if (error != vr::VRRenderModelError_None) {
        vr::VRRenderModels()->FreeRenderModel(model);
        logger.error("magma.vulkan.vr-render-target") << "Unable to load VR device render model's texture." << error << std::endl;
    }

    // Construct Mesh
    auto& mesh = scene.make<magma::Mesh>();
    m_devicesMeshes[deviceType] = &mesh;

    std::vector<glm::vec3> positions(model->unVertexCount);
    std::vector<glm::vec3> normals(model->unVertexCount);
    std::vector<glm::vec2> uvs(model->unVertexCount);
    for (auto i = 0u; i < model->unVertexCount; ++i) {
        const auto& vertexData = model->rVertexData[i];
        positions[i].x = vertexData.vPosition.v[0];
        positions[i].y = vertexData.vPosition.v[1];
        positions[i].z = vertexData.vPosition.v[2];
        normals[i].x = vertexData.vNormal.v[0];
        normals[i].y = vertexData.vNormal.v[1];
        normals[i].z = vertexData.vNormal.v[2];
        uvs[i].x = vertexData.rfTextureCoord[0];
        uvs[i].y = vertexData.rfTextureCoord[1];
    }

    // @fixme We could memcpy, as this is already uint16_t[]
    std::vector<uint16_t> indices(3 * model->unTriangleCount);
    for (auto i = 0u; i < 3 * model->unTriangleCount; ++i) {
        indices[i] = model->rIndexData[i];
    }

    // Add material
    auto& material = scene.make<Material>("vr");
    auto& diffuseTexture = scene.make<Texture>();
    diffuseTexture.loadFromMemory(texture->rubTextureMapData, texture->unWidth, texture->unHeight, 4u);
    material.set("diffuseMap", diffuseTexture);

    mesh.verticesCount(model->unVertexCount);
    mesh.verticesPositions(positions);
    mesh.verticesNormals(normals);
    mesh.indices(indices);
    mesh.material(material);
    mesh.vrRenderable(deviceType != VrDeviceType::Head);

    // Clean up
    vr::VRRenderModels()->FreeRenderModel(model);
    vr::VRRenderModels()->FreeTexture(texture);

    logger.log().tab(-1);
    return mesh;
}

std::optional<VrEvent> VrEngine::pollEvent()
{
    if (m_vrSystem == nullptr) return std::nullopt;

    vr::VREvent_t event;
    if (m_vrSystem->PollNextEvent(&event, sizeof(vr::VREvent_t))) {
        if (event.eventType == vr::VREvent_ButtonPress) {
            VrEvent vrEvent;
            vrEvent.type = VrEventType::ButtonPressed;
            vrEvent.button.hand = deviceTypeHandFromOpenVrDeviceIndex(*m_vrSystem, event.trackedDeviceIndex);
            vrEvent.button.which = buttonFromOpenVrButtonId(event.data.controller.button);
            return vrEvent;
        }
        else if (event.eventType == vr::VREvent_ButtonUnpress) {
            VrEvent vrEvent;
            vrEvent.type = VrEventType::ButtonReleased;
            vrEvent.button.hand = deviceTypeHandFromOpenVrDeviceIndex(*m_vrSystem, event.trackedDeviceIndex);
            vrEvent.button.which = buttonFromOpenVrButtonId(event.data.controller.button);
            return vrEvent;
        }
    }

    return std::nullopt;
}
