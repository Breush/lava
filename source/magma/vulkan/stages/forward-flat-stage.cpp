#include "./forward-flat-stage.hpp"

#include <lava/magma/camera.hpp>
#include <lava/magma/flat.hpp>
#include <lava/magma/scene.hpp>

#include "../../aft-vulkan/camera-aft.hpp"
#include "../../aft-vulkan/flat-aft.hpp"
#include "../../aft-vulkan/scene-aft.hpp"
#include "../helpers/format.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

ForwardFlatStage::ForwardFlatStage(Scene& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine().impl())
    , m_pipelineHolder(m_scene.engine().impl())
    , m_finalImageHolder(m_scene.engine().impl(), "magma.vulkan.stages.forward-flat.final-image")
    , m_framebuffer(m_scene.engine().impl().device())
{
}

void ForwardFlatStage::init(const Camera& camera)
{
    m_camera = &camera;

    logger.info("magma.vulkan.stages.forward-flat") << "Initializing." << std::endl;
    logger.log().tab(1);

    updatePassShaders(true);
    initPass();

    //----- Render pass

    m_renderPassHolder.add(m_pipelineHolder);

    logger.log().tab(-1);
}

void ForwardFlatStage::rebuild()
{
    if (m_rebuildRenderPass) {
        m_renderPassHolder.init();
        m_rebuildRenderPass = false;
    }

    if (m_rebuildPipelines) {
        m_pipelineHolder.update(m_extent);
        m_rebuildPipelines = false;
    }

    if (m_rebuildResources) {
        createResources();
        createFramebuffers();
        m_rebuildResources = false;
    }
}

void ForwardFlatStage::render(vk::CommandBuffer commandBuffer, uint32_t /* frameId */)
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    const auto& deviceHolder = m_scene.engine().impl().deviceHolder();
    deviceHolder.debugBeginRegion(commandBuffer, "forward-flat");

    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 1> clearValues;
    std::array<float, 4> clearColor{0.f, 0.f, 0.f, 0.f};
    clearValues[0u].color = vk::ClearColorValue(clearColor);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPassHolder.renderPass();
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-flat.pass");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelineHolder.pipeline());

    // Set the camera
    m_camera->aft().render(commandBuffer, m_pipelineHolder.pipelineLayout(), CAMERA_PUSH_CONSTANT_OFFSET);

    // Draw all flats
    for (auto flat : m_scene.flats()) {
        tracker.counter("draw-calls.flat-renderer") += 1u;

        flat->aft().render(commandBuffer, m_pipelineHolder.pipelineLayout(), FLAT_PUSH_CONSTANT_OFFSET, MATERIAL_DESCRIPTOR_SET_INDEX);
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugEndRegion(commandBuffer);
}

void ForwardFlatStage::extent(vk::Extent2D extent)
{
    if (m_extent == extent) return;
    m_extent = extent;
    m_rebuildPipelines = true;
    m_rebuildResources = true;
}

RenderImage ForwardFlatStage::renderImage() const
{
    auto cameraId = m_scene.aft().cameraId(*m_camera);
    return m_finalImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA + cameraId);
}

RenderImage ForwardFlatStage::depthRenderImage() const
{
    // No depth image!
    return RenderImage();
}

void ForwardFlatStage::changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    m_finalImageHolder.changeLayout(imageLayout, commandBuffer);
}

//----- Internal

void ForwardFlatStage::initPass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_pipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());

    //----- Push constants

    m_pipelineHolder.addPushConstantRange(sizeof(CameraUbo));
    m_pipelineHolder.addPushConstantRange(sizeof(FlatUbo));

    //----- Rasterization

    m_pipelineHolder.set(vk::CullModeFlagBits::eNone);

    //----- Attachments

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    finalColorAttachment.blending = vulkan::PipelineHolder::ColorAttachmentBlending::AlphaBlending;
    m_pipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(FlatVertex);
    vertexInput.attributes = {{vk::Format::eR32G32Sfloat, offsetof(FlatVertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(FlatVertex, uv)}};
    m_pipelineHolder.set(vertexInput);
}

void ForwardFlatStage::updatePassShaders(bool firstTime)
{
    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["USE_CAMERA_PUSH_CONSTANT"] = "1";
    moduleOptions.defines["USE_FLAT_PUSH_CONSTANT"] = "1";
    moduleOptions.defines["USE_MESH_PUSH_CONSTANT"] = "0";
    moduleOptions.defines["USE_SHADOW_MAP_PUSH_CONSTANT"] = "0";
    moduleOptions.defines["MATERIAL_DESCRIPTOR_SET_INDEX"] = std::to_string(MATERIAL_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MATERIAL_DATA_SIZE"] = std::to_string(MATERIAL_DATA_SIZE);
    moduleOptions.defines["MATERIAL_SAMPLERS_SIZE"] = std::to_string(MATERIAL_SAMPLERS_SIZE);
    if (firstTime) moduleOptions.updateCallback = [this]() { updatePassShaders(false); };

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/renderers/flat/flat.vert", moduleOptions);
    auto fragmentShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/renderers/flat/flat.frag", moduleOptions);

    m_pipelineHolder.removeShaderStages();
    m_pipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_pipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    if (!firstTime) {
        m_pipelineHolder.update(m_extent);
    }
}

void ForwardFlatStage::createResources()
{
    // Final
    auto finalFormat = vk::Format::eR8G8B8A8Unorm;
    m_finalImageHolder.create(finalFormat, m_extent, vk::ImageAspectFlagBits::eColor);
}

void ForwardFlatStage::createFramebuffers()
{
    // Attachments
    std::vector<vk::ImageView> attachments;
    attachments.emplace_back(m_finalImageHolder.view());

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
        logger.error("magma.vulkan.stages.forward-flat") << "Failed to create framebuffers." << std::endl;
    }
}
