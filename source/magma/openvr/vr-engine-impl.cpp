#include "./vr-engine-impl.hpp"

#include <lava/magma/material.hpp>
#include <lava/magma/scene.hpp>
#include <lava/magma/texture.hpp>

using namespace lava;
using namespace lava::chamber;
using namespace lava::magma;

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

VrEngine::Impl::Impl(VrEngine& engine)
    : m_engine(engine)
    , m_devicesPoses(vr::k_unMaxTrackedDeviceCount)
{
}

bool VrEngine::Impl::init()
{
    if (!vr::VR_IsHmdPresent()) {
        logger.info("magma.openvr.vr-engine") << "VR is not available." << std::endl;
        return false;
    }

    // Initializing VR system
    vr::EVRInitError error = vr::VRInitError_None;
    m_vrSystem = vr::VR_Init(&error, vr::EVRApplicationType::VRApplication_Scene);

    if (error != vr::VRInitError_None) {
        logger.warning("magma.openvr.vr-engine") << "VR seems available but we failed to init VR. Is SteamVR ready?" << std::endl;
        return false;
    }

    logger.info("magma.openvr.vr-engine") << "VR enabled." << std::endl;
    return true;
}

void VrEngine::Impl::update()
{
    m_vrSystem->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0.f, m_devicesPoses.data(), vr::k_unMaxTrackedDeviceCount);

    const auto& areaTransform = m_engine.transform();
    for (auto deviceIndex = 0u; deviceIndex < vr::k_unMaxTrackedDeviceCount; ++deviceIndex) {
        const auto& pose = m_devicesPoses[deviceIndex];

        if (!pose.bPoseIsValid) continue;

        auto type = m_vrSystem->GetTrackedDeviceClass(deviceIndex);
        if (type == vr::TrackedDeviceClass_TrackingReference) continue;

        DeviceInfo deviceInfo;
        deviceInfo.valid = pose.bPoseIsValid;

        auto m = pose.mDeviceToAbsoluteTracking.m;
        auto deviceTransform = glm::mat4(m[0][0], m[1][0], m[2][0], 0.f, // X
                                         m[0][1], m[1][1], m[2][1], 0.f, // Y
                                         m[0][2], m[1][2], m[2][2], 0.f, // Z
                                         m[0][3], m[1][3], m[2][3], 1.f);
        // @fixme Might not be our job to multiply by areaTransform
        deviceInfo.transform = areaTransform * m_fixesTransform * deviceTransform;

        deviceInfo.data[0u] = deviceIndex;
        deviceInfo.data[1u] = type;

        if (type == vr::TrackedDeviceClass_HMD) {
            m_engine.deviceInfo(VrDeviceType::Head) = deviceInfo;
        }
        else if (type == vr::TrackedDeviceClass_Controller) {
            auto deviceType = deviceTypeHandFromOpenVrDeviceIndex(*m_vrSystem, deviceIndex);
            m_engine.deviceInfo(deviceType) = deviceInfo;
        }
        else {
            logger.warning("magma.openvr.vr-engine") << "Unknown VR device type." << std::endl;
        }
    }
}

std::optional<VrEvent> VrEngine::Impl::pollEvent()
{
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

VrRenderingNeeds VrEngine::Impl::renderingNeeds(VrRenderingNeedsInfo info) const
{
    if (info.type == VrRenderingNeedsType::Vulkan) {
        static std::string instanceExtensionsString;
        static std::string deviceExtensionsString;

        if (instanceExtensionsString.empty()) {
            uint32_t bufferSize = vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(nullptr, 0);
            instanceExtensionsString.resize(bufferSize);
            vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(instanceExtensionsString.data(), bufferSize);
            std::replace(instanceExtensionsString.begin(), instanceExtensionsString.end(), ' ', '\0');
        }

        if (info.vulkan.physicalDevice != nullptr && deviceExtensionsString.empty()) {
            uint32_t bufferSize = vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(info.vulkan.physicalDevice, nullptr, 0);
            deviceExtensionsString.resize(bufferSize);
            vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(info.vulkan.physicalDevice, deviceExtensionsString.data(), bufferSize);
            std::replace(deviceExtensionsString.begin(), deviceExtensionsString.end(), ' ', '\0');
        }

        auto instanceExtensions = splitAsViews(instanceExtensionsString, '\0');
        auto deviceExtensions = splitAsViews(deviceExtensionsString, '\0');

        VrRenderingNeeds needs;
        needs.vulkan.instanceExtensionCount = instanceExtensions.size();
        needs.vulkan.deviceExtensionCount = deviceExtensions.size();
        for (auto i = 0u; i < instanceExtensions.size(); ++i) {
            needs.vulkan.instanceExtensionsNames[i] = instanceExtensions[i].data();
        }
        for (auto i = 0u; i < deviceExtensions.size(); ++i) {
            needs.vulkan.deviceExtensionsNames[i] = deviceExtensions[i].data();
        }

        return needs;
    }

    // Unhandled back-end
    logger.warning("magma.openvr.vr-engine") << "Unknown VR rendering needs type." << std::endl;
    return VrRenderingNeeds();
}

// ----- Devices

Mesh& VrEngine::Impl::deviceMesh(VrDeviceType deviceType, Scene& scene) const
{
    // Get device name
    auto deviceIndex = m_engine.deviceInfo(deviceType).data[0u];
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
        logger.error("magma.openvr.vr-engine") << "Unable to load VR device render model. " << error << std::endl;
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
        logger.error("magma.openvr.vr-engine") << "Unable to load VR device render model's texture." << error << std::endl;
    }

    // Construct Mesh
    auto& mesh = scene.make<magma::Mesh>();

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

    // @note Using memcpy, as this is already uint16_t[]
    std::vector<uint16_t> indices(3 * model->unTriangleCount);
    memcpy(indices.data(), model->rIndexData, indices.size() * sizeof(uint16_t));

    // Add material
    auto material = scene.makeMaterial("vr");
    auto diffuseTexture = scene.makeTexture();
    diffuseTexture->loadFromMemory(texture->rubTextureMapData, texture->unWidth, texture->unHeight, 4u);
    material->set("diffuseMap", diffuseTexture);

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

// ----- View

Extent2d VrEngine::Impl::renderTargetExtent() const
{
    Extent2d extent;
    m_vrSystem->GetRecommendedRenderTargetSize(&extent.width, &extent.height);
    return extent;
}

glm::mat4 VrEngine::Impl::eyeProjectionTransform(VrEye eye, float nearClip, float farClip) const
{
    vr::HmdMatrix44_t mat =
        m_vrSystem->GetProjectionMatrix((eye == VrEye::Left) ? vr::Eye_Left : vr::Eye_Right, nearClip, farClip);
    auto transform =
        glm::mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0], mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
                  mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
    transform[1][1] *= -1;
    return transform;
}

glm::mat4 VrEngine::Impl::eyeToHeadTransform(VrEye eye) const
{
    auto mat = m_vrSystem->GetEyeToHeadTransform((eye == VrEye::Left) ? vr::Eye_Left : vr::Eye_Right);
    auto transform = glm::mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.f, mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.f,
                               mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.f, mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.f);
    return transform;
}
