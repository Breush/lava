#include "./deep-deferred-stage.hpp"

#include <lava/chamber/logger.hpp>

#include "../cameras/i-camera-impl.hpp"
#include "../lights/i-light-impl.hpp"
#include "../lights/point-light-impl.hpp"
#include "../meshes/i-mesh-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../vertex.hpp"

namespace {
    constexpr const auto GBUFFER_MAX_NODE_DEPTH = 3u;
    constexpr const auto GBUFFER_MAX_NODE_DEPTH_STRING = "3";

    struct EpiphanyPointLightUbo {
        glm::vec4 wPosition;
        float radius;
    };

    vk::Format findSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates,
                                   vk::ImageTiling tiling, vk::FormatFeatureFlags features)
    {
        for (auto format : candidates) {
            vk::FormatProperties props = physicalDevice.getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        return vk::Format::eUndefined;
    }

    vk::Format findDepthBufferFormat(vk::PhysicalDevice physicalDevice)
    {
        return findSupportedFormat(physicalDevice,
                                   {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                   vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }
}

using namespace lava::magma;
using namespace lava::chamber;

DeepDeferredStage::DeepDeferredStage(RenderScene::Impl& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine())
    , m_clearPipelineHolder(m_scene.engine())
    , m_geometryOpaquePipelineHolder(m_scene.engine())
    , m_geometryTranslucentPipelineHolder(m_scene.engine())
    , m_epiphanyPipelineHolder(m_scene.engine())
    , m_epiphanyDescriptorHolder(m_scene.engine())
    , m_epiphanyUboHolder(m_scene.engine())
    , m_gBufferDescriptorHolder(m_scene.engine())
    , m_gBufferHeaderBufferHolder(m_scene.engine())
    , m_gBufferListBufferHolder(m_scene.engine())
    , m_finalImageHolder(m_scene.engine())
    , m_depthImageHolder(m_scene.engine())
    , m_framebuffer(m_scene.engine().device())
{
}

void DeepDeferredStage::init(uint32_t cameraId)
{
    m_cameraId = cameraId;

    logger.info("magma.vulkan.stages.deep-deferred-stage") << "Initializing." << std::endl;
    logger.log().tab(1);

    initGBuffer();
    initClearPass();
    initGeometryOpaquePass();
    initGeometryTranslucentPass();
    initEpiphanyPass();

    //----- Render pass

    m_renderPassHolder.add(m_clearPipelineHolder);
    m_renderPassHolder.add(m_geometryOpaquePipelineHolder);
    m_renderPassHolder.add(m_geometryTranslucentPipelineHolder);
    m_renderPassHolder.add(m_epiphanyPipelineHolder);
    m_renderPassHolder.init();

    logger.log().tab(-1);
}

void DeepDeferredStage::update(vk::Extent2D extent)
{
    m_extent = extent;

    m_clearPipelineHolder.update(extent);
    m_geometryOpaquePipelineHolder.update(extent);
    m_geometryTranslucentPipelineHolder.update(extent);
    m_epiphanyPipelineHolder.update(extent);

    createResources();
    createFramebuffers();
}

void DeepDeferredStage::render(vk::CommandBuffer commandBuffer)
{
    const auto& deviceHolder = m_scene.engine().deviceHolder();
    deviceHolder.debugMarkerBeginRegion(commandBuffer, "DeepDeferredStage");

    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].depthStencil = vk::ClearDepthStencilValue{1.f, 0u};
    // @note Not important - clearValues[1].color;

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPassHolder.renderPass();
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Clear pass

    deviceHolder.debugMarkerBeginRegion(commandBuffer, "Clear pass");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_clearPipelineHolder.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_clearPipelineHolder.pipelineLayout(),
                                     GBUFFER_DESCRIPTOR_SET_INDEX, 1, &m_gBufferDescriptorSet, 0, nullptr);
    commandBuffer.draw(6, 1, 0, 0);

    deviceHolder.debugMarkerEndRegion(commandBuffer);

    //----- Geometry opaque pass

    deviceHolder.debugMarkerBeginRegion(commandBuffer, "Geometry opaque pass");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_geometryOpaquePipelineHolder.pipeline());

    // Set the camera
    auto& camera = m_scene.camera(m_cameraId);
    camera.render(commandBuffer, m_geometryTranslucentPipelineHolder.pipelineLayout(), CAMERA_DESCRIPTOR_SET_INDEX);

    // Draw all opaque meshes
    for (auto& mesh : m_scene.meshes()) {
        if (!mesh->translucent()) {
            mesh->interfaceImpl().render(commandBuffer, m_geometryTranslucentPipelineHolder.pipelineLayout(),
                                         MESH_DESCRIPTOR_SET_INDEX);
            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
                                          vk::DependencyFlags(), 0, nullptr, 0, nullptr, 0, nullptr);
        }
    }

    deviceHolder.debugMarkerEndRegion(commandBuffer);

    //----- Geometry translucent pass

    deviceHolder.debugMarkerBeginRegion(commandBuffer, "Geometry translucent pass");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_geometryTranslucentPipelineHolder.pipeline());

    // Draw all translucent meshes
    for (auto& mesh : m_scene.meshes()) {
        if (mesh->translucent()) {
            mesh->interfaceImpl().render(commandBuffer, m_geometryTranslucentPipelineHolder.pipelineLayout(),
                                         MESH_DESCRIPTOR_SET_INDEX);
            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
                                          vk::DependencyFlags(), 0, nullptr, 0, nullptr, 0, nullptr);
        }
    }

    deviceHolder.debugMarkerEndRegion(commandBuffer);

    //----- Epiphany pass

    deviceHolder.debugMarkerBeginRegion(commandBuffer, "Epiphany pass");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_epiphanyPipelineHolder.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_epiphanyPipelineHolder.pipelineLayout(),
                                     EPIPHANY_DESCRIPTOR_SET_INDEX, 1, &m_epiphanyDescriptorSet, 0, nullptr);

    // @todo With LLL subpass, this shouldn't be needed anymore
    updateEpiphanyUbo();

    commandBuffer.draw(6, 1, 0, 1);

    deviceHolder.debugMarkerEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugMarkerEndRegion(commandBuffer);
}

//----- Internal

void DeepDeferredStage::initGBuffer()
{
    m_gBufferDescriptorHolder.storageBufferSizes({1, 1});
    m_gBufferDescriptorHolder.init(1, vk::ShaderStageFlagBits::eFragment);
    m_gBufferDescriptorSet = m_gBufferDescriptorHolder.allocateSet();
}

void DeepDeferredStage::initClearPass()
{
    //----- Shaders

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/fullscreen.vert");
    m_clearPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    auto fragmentShaderModule = m_scene.engine().shadersManager().module(
        "./data/shaders/stages/deep-deferred-clear.frag", {{"GBUFFER_MAX_NODE_DEPTH", GBUFFER_MAX_NODE_DEPTH_STRING}});
    m_clearPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    //----- Descriptor set layouts

    m_clearPipelineHolder.add(m_gBufferDescriptorHolder.setLayout());
}

void DeepDeferredStage::initGeometryOpaquePass()
{
    // Allow using a pipelineBarrier during the rendering of the pass.
    m_geometryOpaquePipelineHolder.selfDependent(true);

    //----- Shaders

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/deep-deferred-geometry.vert");
    m_geometryOpaquePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    auto fragmentShaderModule = m_scene.engine().shadersManager().module(
        "./data/shaders/stages/deep-deferred-geometry-opaque.frag", {{"GBUFFER_MAX_NODE_DEPTH", GBUFFER_MAX_NODE_DEPTH_STRING}});
    m_geometryOpaquePipelineHolder.add(
        {shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    //----- Descriptor set layouts

    // @note Ordering is important
    m_geometryOpaquePipelineHolder.add(m_gBufferDescriptorHolder.setLayout());
    m_geometryOpaquePipelineHolder.add(m_scene.cameraDescriptorHolder().setLayout());
    m_geometryOpaquePipelineHolder.add(m_scene.materialDescriptorHolder().setLayout());
    m_geometryOpaquePipelineHolder.add(m_scene.meshDescriptorHolder().setLayout());

    //----- Rasterization

    m_geometryOpaquePipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = findDepthBufferFormat(m_scene.engine().physicalDevice());
    m_geometryOpaquePipelineHolder.set(depthStencilAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(vulkan::Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(vulkan::Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(vulkan::Vertex, tangent)}};
    m_geometryOpaquePipelineHolder.set(vertexInput);
}

void DeepDeferredStage::initGeometryTranslucentPass()
{
    // Allow using a pipelineBarrier during the rendering of the pass.
    m_geometryTranslucentPipelineHolder.selfDependent(true);

    //----- Shaders

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/deep-deferred-geometry.vert");
    m_geometryTranslucentPipelineHolder.add(
        {shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/deep-deferred-geometry-translucent.frag",
                                                 {{"GBUFFER_MAX_NODE_DEPTH", GBUFFER_MAX_NODE_DEPTH_STRING}});
    m_geometryTranslucentPipelineHolder.add(
        {shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    //----- Descriptor set layouts

    // @note Ordering is important
    m_geometryTranslucentPipelineHolder.add(m_gBufferDescriptorHolder.setLayout());
    m_geometryTranslucentPipelineHolder.add(m_scene.cameraDescriptorHolder().setLayout());
    m_geometryTranslucentPipelineHolder.add(m_scene.materialDescriptorHolder().setLayout());
    m_geometryTranslucentPipelineHolder.add(m_scene.meshDescriptorHolder().setLayout());

    //----- Rasterization

    m_geometryTranslucentPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(vulkan::Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(vulkan::Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(vulkan::Vertex, tangent)}};
    m_geometryTranslucentPipelineHolder.set(vertexInput);
}

void DeepDeferredStage::initEpiphanyPass()
{
    //----- Shaders

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/fullscreen.vert");
    m_epiphanyPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    auto fragmentShaderModule = m_scene.engine().shadersManager().module(
        "./data/shaders/stages/deep-deferred-epiphany.frag", {{"GBUFFER_MAX_NODE_DEPTH", GBUFFER_MAX_NODE_DEPTH_STRING}});
    m_epiphanyPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    //----- Descriptors

    m_epiphanyPipelineHolder.add(m_gBufferDescriptorHolder.setLayout());
    m_epiphanyPipelineHolder.add(m_scene.cameraDescriptorHolder().setLayout());

    // UBO: EpiphanyLightUbo
    m_epiphanyDescriptorHolder.uniformBufferSizes({1});
    m_epiphanyDescriptorHolder.init(1, vk::ShaderStageFlagBits::eFragment);
    m_epiphanyPipelineHolder.add(m_epiphanyDescriptorHolder.setLayout());

    m_epiphanyDescriptorSet = m_epiphanyDescriptorHolder.allocateSet();

    //----- Uniform buffers

    m_epiphanyUboHolder.init(m_epiphanyDescriptorSet, m_epiphanyDescriptorHolder.uniformBufferBindingOffset(),
                             {sizeof(EpiphanyPointLightUbo)});

    //----- Attachments

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    m_epiphanyPipelineHolder.add(finalColorAttachment);
}

void DeepDeferredStage::createResources()
{
    // Final
    auto finalFormat = vk::Format::eR8G8B8A8Unorm;
    m_finalImageHolder.create(finalFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Depth
    auto depthFormat = findDepthBufferFormat(m_scene.engine().physicalDevice());
    m_depthImageHolder.create(depthFormat, m_extent, vk::ImageAspectFlagBits::eDepth);

    // GBuffer
    vk::DeviceSize gBufferHeaderSize = 1u * sizeof(uint32_t) + m_extent.width * m_extent.height * sizeof(uint32_t);
    m_gBufferHeaderBufferHolder.create(vk::BufferUsageFlagBits::eStorageBuffer, gBufferHeaderSize);
    m_gBufferDescriptorHolder.updateSet(m_gBufferDescriptorSet, m_gBufferHeaderBufferHolder.buffer(), gBufferHeaderSize, 0);
    m_gBufferHeaderBufferHolder.copy(m_extent.width);

    vk::DeviceSize gBufferListSize =
        1u * sizeof(uint32_t) + GBUFFER_MAX_NODE_DEPTH * m_extent.width * m_extent.height * sizeof(GBufferNode);
    m_gBufferListBufferHolder.create(vk::BufferUsageFlagBits::eStorageBuffer, gBufferListSize);
    m_gBufferDescriptorHolder.updateSet(m_gBufferDescriptorSet, m_gBufferListBufferHolder.buffer(), gBufferListSize, 1);

    logger.info("magma.vulkan.stages.deep-deferred-stage") << "GBuffer sizes | header: " << gBufferHeaderSize / 1000000.f
                                                           << "Mo | list: " << gBufferListSize / 1000000.f << "Mo." << std::endl;
}

void DeepDeferredStage::createFramebuffers()
{
    // Framebuffer
    std::array<vk::ImageView, 2> attachments = {m_depthImageHolder.view(), m_finalImageHolder.view()};

    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPassHolder.renderPass();
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_scene.engine().device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.deep-deferred-stage") << "Failed to create framebuffers." << std::endl;
    }
}

void DeepDeferredStage::updateEpiphanyUbo()
{
    if (m_scene.lights().size() > 0) {
        const auto& light = m_scene.light(0);

        if (light.type() == LightType::Point) {
            const auto& pointLight = reinterpret_cast<const PointLight::Impl&>(light);

            EpiphanyPointLightUbo ubo;
            ubo.wPosition = glm::vec4(pointLight.position(), 1.f);
            ubo.radius = pointLight.radius();

            m_epiphanyUboHolder.copy(0, ubo);
        }
    }
}
