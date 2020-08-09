#include "./forward-renderer-stage.hpp"

#include <lava/magma/camera.hpp>
#include <lava/magma/light.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/scene.hpp>
#include <lava/magma/vertex.hpp>

#include "../../aft-vulkan/camera-aft.hpp"
#include "../../aft-vulkan/light-aft.hpp"
#include "../../aft-vulkan/mesh-aft.hpp"
#include "../../aft-vulkan/scene-aft.hpp"
#include "../../g-buffer-data.hpp"
#include "../environment.hpp"
#include "../helpers/format.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

ForwardRendererStage::ForwardRendererStage(Scene& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine().impl())
    , m_opaquePipelineHolder(m_scene.engine().impl())
    , m_maskPipelineHolder(m_scene.engine().impl())
    , m_depthlessPipelineHolder(m_scene.engine().impl())
    , m_wireframePipelineHolder(m_scene.engine().impl())
    , m_translucentPipelineHolder(m_scene.engine().impl())
    , m_finalImageHolder(m_scene.engine().impl(), "stages.forward-renderer.final")
    , m_finalResolveImageHolder(m_scene.engine().impl(), "stages.forward-renderer.final-resolve")
    , m_depthImageHolder(m_scene.engine().impl(), "stages.forward-renderer.depth")
    , m_framebuffer(m_scene.engine().impl().device())
{
}

void ForwardRendererStage::init(const Camera& camera)
{
    m_camera = &camera;

    logger.info("magma.vulkan.stages.forward-renderer-stage") << "Initializing." << std::endl;
    logger.log().tab(1);

    updatePassShaders(true);
    initOpaquePass();
    initMaskPass();
    initDepthlessPass();
    initWireframePass();
    initTranslucentPass();

    //----- Render pass

    m_renderPassHolder.add(m_opaquePipelineHolder);
    m_renderPassHolder.add(m_maskPipelineHolder);
    m_renderPassHolder.add(m_depthlessPipelineHolder);
    m_renderPassHolder.add(m_wireframePipelineHolder);
    m_renderPassHolder.add(m_translucentPipelineHolder);

    logger.log().tab(-1);
}

void ForwardRendererStage::rebuild()
{
    if (m_rebuildRenderPass) {
        m_renderPassHolder.init();
        m_rebuildRenderPass = false;
    }

    if (m_rebuildPipelines) {
        m_opaquePipelineHolder.update(m_extent, m_polygonMode);
        m_maskPipelineHolder.update(m_extent, m_polygonMode);
        m_depthlessPipelineHolder.update(m_extent, m_polygonMode);
        m_wireframePipelineHolder.update(m_extent, vk::PolygonMode::eLine);
        m_translucentPipelineHolder.update(m_extent, m_polygonMode);
        m_rebuildPipelines = false;
    }

    if (m_rebuildResources) {
        createResources();
        createFramebuffers();
        m_rebuildResources = false;
    }
}

void ForwardRendererStage::record(vk::CommandBuffer commandBuffer, uint32_t frameId)
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    const auto& deviceHolder = m_scene.engine().impl().deviceHolder();
    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer");

    auto cameraMatrix = m_camera->projectionMatrix() * m_camera->viewMatrix();

    //----- Prologue

    // @todo These clearValues could be returned by a method of RenderPassHolder,
    // making sure it has the right size and with default values already set
    // during construction.

    // Set render pass
    std::array<vk::ClearValue, 2> clearValues;
    // @fixme Allow clear color to be configurable per scene or camera!
    std::array<float, 4> clearColor{1.f, 1.f, 1.f, 0.f};
    clearValues[0u].color = vk::ClearColorValue(clearColor);
    clearValues[1u].depthStencil = vk::ClearDepthStencilValue{1.f, 0u};

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPassHolder.renderPass();
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Opaque pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.opaque");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_opaquePipelineHolder.pipeline());

    // Bind material global
    auto materialGlobalDescriptorSet = m_scene.aft().materialGlobalDescriptorSet();
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_opaquePipelineHolder.pipelineLayout(),
                                     MATERIAL_GLOBAL_DESCRIPTOR_SET_INDEX, 1u, &materialGlobalDescriptorSet, 0u, nullptr);

    // Bind lights
    for (auto light : m_scene.lights()) {
        light->aft().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), LIGHTS_DESCRIPTOR_SET_INDEX);
        m_scene.aft()
            .shadows(*light, *m_camera)
            .render(commandBuffer, frameId, m_opaquePipelineHolder.pipelineLayout(), SHADOWS_DESCRIPTOR_SET_INDEX);
    }

    // Set the camera
    m_camera->aft().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), CAMERA_PUSH_CONSTANT_OFFSET);
    const auto& cameraFrustum = m_camera->frustum();

    // Set the environment
    m_scene.aft().environment().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), ENVIRONMENT_DESCRIPTOR_SET_INDEX);

    // Draw all opaque meshes and sort others
    struct TranslucentMesh {
        const Mesh* mesh;
        float distanceToCamera;
    };

    std::vector<const Mesh*> maskMeshes;
    std::vector<const Mesh*> depthlessMeshes;
    std::vector<const Mesh*> wireframedMeshes;
    std::vector<TranslucentMesh> translucentMeshes;
    for (auto mesh : m_scene.meshes()) {
        if (m_camera->vrAimed() && !mesh->vrRenderable()) continue;

        auto category = mesh->category();
        if (category == RenderCategory::Depthless) {
            depthlessMeshes.emplace_back(mesh);
            continue;
        }

        const auto& boundingSphere = mesh->boundingSphere();
        if (!m_camera->frustumCullingEnabled() || cameraFrustum.canSee(boundingSphere)) {
            if (category == RenderCategory::Mask) {
                maskMeshes.emplace_back(mesh);
                continue;
            }
            else if (category == RenderCategory::Translucent) {
                auto distanceToCamera = (cameraMatrix * glm::vec4(boundingSphere.center, 1.f)).z + boundingSphere.radius;
                translucentMeshes.emplace_back(TranslucentMesh{mesh, distanceToCamera});
                continue;
            }
            else if (category == RenderCategory::Wireframe) {
                wireframedMeshes.emplace_back(mesh);
                continue;
            }

            tracker.counter("draw-calls.renderer") += 1u;
            mesh->aft().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET,
                               MATERIAL_DESCRIPTOR_SET_INDEX);
        }
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Mask pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.mask");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_maskPipelineHolder.pipeline());

    for (auto mesh : maskMeshes) {
        tracker.counter("draw-calls.renderer") += 1u;
        mesh->aft().render(commandBuffer, m_maskPipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET,
                           MATERIAL_DESCRIPTOR_SET_INDEX);
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Depthless pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.depthless");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_depthlessPipelineHolder.pipeline());

    for (auto mesh : depthlessMeshes) {
        tracker.counter("draw-calls.renderer") += 1u;
        mesh->aft().render(commandBuffer, m_depthlessPipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET,
                           MATERIAL_DESCRIPTOR_SET_INDEX);
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Wireframe pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.wireframe");

    // @todo No need to bind wireframe if there is nothing to draw in it.
    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_wireframePipelineHolder.pipeline());

    // Draw all wireframed meshes
    for (auto mesh : wireframedMeshes) {
        tracker.counter("draw-calls.renderer") += 1u;
        mesh->aft().renderUnlit(commandBuffer, m_wireframePipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET);
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Translucent pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.translucent");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_translucentPipelineHolder.pipeline());

    // Draw all translucent meshes
    std::sort(translucentMeshes.begin(), translucentMeshes.end(), [](const TranslucentMesh& a, const TranslucentMesh& b) {
        return a.distanceToCamera > b.distanceToCamera;
    });

    for (auto translucentMesh : translucentMeshes) {
        tracker.counter("draw-calls.renderer") += 1u;
        translucentMesh.mesh->aft().render(commandBuffer, m_translucentPipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET,
                                           MATERIAL_DESCRIPTOR_SET_INDEX);
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugEndRegion(commandBuffer);
}

void ForwardRendererStage::extent(const vk::Extent2D& extent)
{
    if (m_extent == extent) return;
    m_extent = extent;
    m_rebuildPipelines = true;
    m_rebuildResources = true;
}

void ForwardRendererStage::polygonMode(vk::PolygonMode polygonMode)
{
    if (m_polygonMode == polygonMode) return;
    m_polygonMode = polygonMode;
    m_rebuildPipelines = true;
}

void ForwardRendererStage::sampleCount(vk::SampleCountFlagBits sampleCount)
{
    if (m_sampleCount == sampleCount) return;
    m_sampleCount = sampleCount;
    m_msaaEnabled = (sampleCount != vk::SampleCountFlagBits::e1);

    m_opaquePipelineHolder.set(sampleCount);
    m_maskPipelineHolder.set(sampleCount);
    m_depthlessPipelineHolder.set(sampleCount);
    m_wireframePipelineHolder.set(sampleCount);
    m_translucentPipelineHolder.set(sampleCount);

    // @note Do that only during the last subpass
    if (m_msaaEnabled) {
        vulkan::PipelineHolder::ResolveAttachment finalResolveAttachment;
        finalResolveAttachment.format = vk::Format::eR8G8B8A8Unorm;
        m_translucentPipelineHolder.set(finalResolveAttachment);
    }
    else {
        m_translucentPipelineHolder.resetResolveAttachment();
    }

    m_rebuildRenderPass = true;
    m_rebuildPipelines = true;
    m_rebuildResources = true;
}

RenderImage ForwardRendererStage::renderImage() const
{
    auto cameraId = m_scene.aft().cameraId(*m_camera);

    if (m_msaaEnabled) {
        return m_finalResolveImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA + cameraId);
    }

    return m_finalImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA + cameraId);
}

RenderImage ForwardRendererStage::depthRenderImage() const
{
    auto cameraId = m_scene.aft().cameraId(*m_camera);

    if (m_msaaEnabled) {
        logger.warning("magma.vulkan.stages.forward-renderer") << "Requesting depth render image with active MSAA is not supported." << std::endl;
        return m_finalResolveImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA + cameraId);
    }

    return m_depthImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA_DEPTH + cameraId);
}

void ForwardRendererStage::changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    if (m_msaaEnabled) {
        m_finalResolveImageHolder.changeLayout(imageLayout, commandBuffer);
    }

    m_finalImageHolder.changeLayout(imageLayout, commandBuffer);
}

//----- Internal

void ForwardRendererStage::initOpaquePass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_opaquePipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.aft().materialGlobalDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

    //----- Push constants

    m_opaquePipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_opaquePipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_opaquePipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    m_opaquePipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    m_opaquePipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}};
    m_opaquePipelineHolder.set(vertexInput);
}

void ForwardRendererStage::initMaskPass()
{
    // @note This is basically a copy of the opaque pass, the main difference is the shader,
    // having no early depth test and discarding below 0.5 alpha fragments.
    // We alpha blend for fragments from 0.5 to 1.0 alpha so that transitions
    // are smooth. But do not expect this to work nicely with important translucency,
    // this should only be used on borders.

    //----- Descriptor set layouts

    // @note Ordering is important
    m_maskPipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_maskPipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());
    m_maskPipelineHolder.add(m_scene.aft().materialGlobalDescriptorHolder().setLayout());
    m_maskPipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_maskPipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

    //----- Push constants

    m_maskPipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_maskPipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_maskPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    depthStencilAttachment.clear = false;
    m_maskPipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    finalColorAttachment.blending = vulkan::PipelineHolder::ColorAttachmentBlending::AlphaBlending;
    finalColorAttachment.clear = false;
    m_maskPipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}};
    m_maskPipelineHolder.set(vertexInput);
}

void ForwardRendererStage::initDepthlessPass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_depthlessPipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_depthlessPipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());
    m_depthlessPipelineHolder.add(m_scene.aft().materialGlobalDescriptorHolder().setLayout());
    m_depthlessPipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_depthlessPipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

    //----- Push constants

    m_depthlessPipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_depthlessPipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_depthlessPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    depthStencilAttachment.depthWriteEnabled = false;
    depthStencilAttachment.clear = false;
    m_depthlessPipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    finalColorAttachment.clear = false;
    m_depthlessPipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}};
    m_depthlessPipelineHolder.set(vertexInput);
}

void ForwardRendererStage::initWireframePass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_wireframePipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_wireframePipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());
    // @note Useful, so that the pipelines are all compatible
    m_wireframePipelineHolder.add(m_scene.aft().materialGlobalDescriptorHolder().setLayout());
    m_wireframePipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_wireframePipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

    //----- Push constants

    m_wireframePipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_wireframePipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_wireframePipelineHolder.set(vk::CullModeFlagBits::eNone);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    depthStencilAttachment.depthWriteEnabled = false;
    depthStencilAttachment.clear = false;
    m_wireframePipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    finalColorAttachment.blending = vulkan::PipelineHolder::ColorAttachmentBlending::AlphaBlending;
    finalColorAttachment.clear = false;
    m_wireframePipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(UnlitVertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(UnlitVertex, pos)}};
    m_wireframePipelineHolder.set(vertexInput);
}

void ForwardRendererStage::initTranslucentPass()
{
    // @note Translucent pass differs from opaque pass just by having
    // depth writing disabled and alpha blending enabled.

    //----- Descriptor set layouts

    // @note Ordering is important
    m_translucentPipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.aft().materialGlobalDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

    //----- Push constants

    m_translucentPipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_translucentPipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_translucentPipelineHolder.set(vk::CullModeFlagBits::eNone);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    depthStencilAttachment.depthWriteEnabled = false;
    depthStencilAttachment.clear = false;
    m_translucentPipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    finalColorAttachment.blending = vulkan::PipelineHolder::ColorAttachmentBlending::AlphaBlending;
    finalColorAttachment.clear = false;
    m_translucentPipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}};
    m_translucentPipelineHolder.set(vertexInput);
}

void ForwardRendererStage::updatePassShaders(bool firstTime)
{
    // @note We use the very same shaders for opaque and translucent meshes,
    // only the depth test and the color attachments differ.

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["USE_CAMERA_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["USE_FLAT_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["USE_MESH_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["USE_SHADOW_MAP_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["MATERIAL_DESCRIPTOR_SET_INDEX"] = std::to_string(MATERIAL_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MATERIAL_GLOBAL_DESCRIPTOR_SET_INDEX"] = std::to_string(MATERIAL_GLOBAL_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["LIGHTS_DESCRIPTOR_SET_INDEX"] = std::to_string(LIGHTS_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["ENVIRONMENT_DESCRIPTOR_SET_INDEX"] = std::to_string(ENVIRONMENT_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT"] = std::to_string(ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT);
    moduleOptions.defines["SHADOWS_DESCRIPTOR_SET_INDEX"] = std::to_string(SHADOWS_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["SHADOWS_CASCADES_COUNT"] = std::to_string(SHADOWS_CASCADES_COUNT);
    moduleOptions.defines["LIGHT_TYPE_POINT"] = std::to_string(static_cast<uint32_t>(LightType::Point));
    moduleOptions.defines["LIGHT_TYPE_DIRECTIONAL"] = std::to_string(static_cast<uint32_t>(LightType::Directional));
    moduleOptions.defines["MATERIAL_DATA_SIZE"] = std::to_string(MATERIAL_DATA_SIZE);
    moduleOptions.defines["MATERIAL_SAMPLERS_SIZE"] = std::to_string(MATERIAL_SAMPLERS_SIZE);
    moduleOptions.defines["MATERIAL_GLOBAL_SAMPLERS_SIZE"] = std::to_string(MATERIAL_SAMPLERS_SIZE);
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    if (firstTime) moduleOptions.updateCallback = [this]() { updatePassShaders(false); };

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry.vert", moduleOptions);
    auto fragmentShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/renderers/forward/geometry.frag", moduleOptions);

    auto maskFragmentShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/renderers/forward/geometry-mask.frag", moduleOptions);

    auto depthlessVertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry-depthless.vert", moduleOptions);

    auto unlitVertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry-unlit.vert", moduleOptions);
    auto unlitFragmentShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry-unlit.frag", moduleOptions);

    m_opaquePipelineHolder.removeShaderStages();
    m_opaquePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_opaquePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    m_maskPipelineHolder.removeShaderStages();
    m_maskPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_maskPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, maskFragmentShaderModule, "main"});

    m_depthlessPipelineHolder.removeShaderStages();
    m_depthlessPipelineHolder.add(
        {shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, depthlessVertexShaderModule, "main"});
    m_depthlessPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    m_wireframePipelineHolder.removeShaderStages();
    m_wireframePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, unlitVertexShaderModule, "main"});
    m_wireframePipelineHolder.add(
        {shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, unlitFragmentShaderModule, "main"});

    m_translucentPipelineHolder.removeShaderStages();
    m_translucentPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_translucentPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    if (!firstTime) {
        m_opaquePipelineHolder.update(m_extent, m_polygonMode);
        m_maskPipelineHolder.update(m_extent, m_polygonMode);
        m_depthlessPipelineHolder.update(m_extent, m_polygonMode);
        m_wireframePipelineHolder.update(m_extent, vk::PolygonMode::eLine);
        m_translucentPipelineHolder.update(m_extent, m_polygonMode);
    }
}

void ForwardRendererStage::createResources()
{
    // Final
    auto finalFormat = vk::Format::eR8G8B8A8Unorm;
    m_finalImageHolder.sampleCount(m_sampleCount);
    m_finalImageHolder.create(finalFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Final resolve
    if (m_msaaEnabled) {
        m_finalResolveImageHolder.create(finalFormat, m_extent, vk::ImageAspectFlagBits::eColor);
    }

    // Depth
    auto depthFormat = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    m_depthImageHolder.sampleCount(m_sampleCount);
    m_depthImageHolder.create(depthFormat, m_extent, vk::ImageAspectFlagBits::eDepth);
}

void ForwardRendererStage::createFramebuffers()
{
    // Attachments
    std::vector<vk::ImageView> attachments;

    for (auto i = 0u; i < 5u; ++i) { // For each subpass
        attachments.emplace_back(m_finalImageHolder.view());
        attachments.emplace_back(m_depthImageHolder.view());
    }

    if (m_msaaEnabled) {
        attachments.emplace_back(m_finalResolveImageHolder.view());
    }

    // Framebuffer
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPassHolder.renderPass();
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_scene.engine().impl().device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace())
        != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.forward-renderer-stage") << "Failed to create framebuffers." << std::endl;
    }
}
