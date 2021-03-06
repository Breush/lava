#include "./shadows-stage.hpp"

#include <lava/magma/scene.hpp>
#include <lava/magma/vertex.hpp>

#include "../../aft-vulkan/mesh-aft.hpp"
#include "../../aft-vulkan/scene-aft.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

void ShadowsStage::Cascade::init(RenderEngine& engine)
{
    imageHolder = std::make_shared<vulkan::ImageHolder>(engine.impl(), "magma.vulkan.stages.shadows.cascade.image");
}

ShadowsStage::ShadowsStage(Scene& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine().impl())
    , m_pipelineHolder(m_scene.engine().impl())
{
}

void ShadowsStage::init(const Light& light)
{
    m_light = &light;

    logger.info("magma.vulkan.stages.shadows") << "Initializing." << std::endl;
    logger.log().tab(1);

    initPass();

    //----- Render pass

    m_renderPassHolder.add(m_pipelineHolder);
    m_renderPassHolder.init();

    logger.log().tab(-1);
}

void ShadowsStage::update(const vk::Extent2D& extent)
{
    m_extent = extent;

    m_pipelineHolder.update(extent);

    createResources();
}

void ShadowsStage::updateFromCamerasCount()
{
    createResources();
}

void ShadowsStage::record(vk::CommandBuffer commandBuffer, const Camera* camera)
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    auto& cascades = m_cascades.at(camera);
    const auto& shadows = m_scene.aft().shadows(*m_light, *camera);
    const auto& deviceHolder = m_scene.engine().impl().deviceHolder();

    for (auto i = 0u; i < SHADOWS_CASCADES_COUNT; ++i) {
        deviceHolder.debugBeginRegion(commandBuffer, "shadows");

        //----- Prologue

        // Set render pass
        std::array<vk::ClearValue, 1> clearValues;
        clearValues[0].depthStencil = vk::ClearDepthStencilValue{2e23, 0u};

        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.renderPass = m_renderPassHolder.renderPass();
        renderPassInfo.framebuffer = cascades[i].framebuffer.get();
        renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
        renderPassInfo.renderArea.extent = m_extent;
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

        //----- Pass

        deviceHolder.debugBeginRegion(commandBuffer, "shadows.pass");

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelineHolder.pipeline());

        cascades[i].ubo.cascadeTransform = shadows.cascadeTransform(i);
        commandBuffer.pushConstants(m_pipelineHolder.pipelineLayout(),
                                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                    SHADOW_MAP_PUSH_CONSTANT_OFFSET, sizeof(ShadowMapUbo), &cascades[i].ubo);

        // @fixme We could frustrum cull out of light's frustrum
        // Draw all meshes
        for (auto mesh : m_scene.meshes()) {
            if (!mesh->shadowsCastable()) continue;
            tracker.counter("draw-calls.shadows") += 1u;
            mesh->aft().renderUnlit(commandBuffer);
        }

        // Draw
        commandBuffer.draw(6, 1, 0, 0);

        deviceHolder.debugEndRegion(commandBuffer);

        //----- Epilogue

        commandBuffer.endRenderPass();

        deviceHolder.debugEndRegion(commandBuffer);
    }
}

RenderImage ShadowsStage::renderImage(const Camera& camera, uint32_t cascadeIndex) const
{
    auto lightId = m_scene.aft().lightId(*m_light);
    return m_cascades.at(&camera)
        .at(cascadeIndex)
        .imageHolder->renderImage(RenderImage::Impl::UUID_CONTEXT_LIGHT_SHADOW_MAP + lightId);
}

//----- Internal

void ShadowsStage::initPass()
{
    //----- Shaders

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["USE_CAMERA_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["USE_FLAT_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["USE_SHADOW_MAP_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["MESH_UNLIT"] = '1';
    moduleOptions.defines["SHADOWS_CASCADES_COUNT"] = std::to_string(SHADOWS_CASCADES_COUNT);

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/shadows.vert", moduleOptions);
    m_pipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    //----- Push constants

    m_pipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_pipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Attachments

    // @todo Ensure that format is supported, and maybe let this be configurable
    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vk::Format::eD16Unorm;
    m_pipelineHolder.set(depthStencilAttachment);

    //----- Rasterization

    m_pipelineHolder.set(vk::CullModeFlagBits::eBack);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(UnlitVertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(UnlitVertex, pos)}};
    m_pipelineHolder.add(vertexInput);

    //----- Instance input

    vertexInput.stride = sizeof(MeshUbo);
    vertexInput.attributes = {{vk::Format::eR32G32B32A32Sfloat, offsetof(MeshUbo, transform0)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(MeshUbo, transform1)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(MeshUbo, transform2)}};
    vertexInput.rate = vk::VertexInputRate::eInstance;
    m_pipelineHolder.add(vertexInput);
}

void ShadowsStage::createResources()
{
    for (auto camera : m_scene.cameras()) {
        ensureResourcesForCamera(*camera);
    }
}

void ShadowsStage::ensureResourcesForCamera(const Camera& camera)
{
    if (m_cascades.find(&camera) != m_cascades.end()) return;

    auto& cascades = m_cascades[&camera];

    // Image
    for (auto i = 0u; i < SHADOWS_CASCADES_COUNT; ++i) {
        cascades[i].init(m_scene.engine());

        auto depthFormat = vk::Format::eD16Unorm;
        cascades[i].imageHolder->create(vulkan::ImageKind::Depth, depthFormat, m_extent);
    }

    // Framebuffer
    for (auto i = 0u; i < SHADOWS_CASCADES_COUNT; ++i) {
        std::array<vk::ImageView, 1> attachments = {cascades[i].imageHolder->view()};

        vk::FramebufferCreateInfo createInfo;
        createInfo.renderPass = m_renderPassHolder.renderPass();
        createInfo.attachmentCount = attachments.size();
        createInfo.pAttachments = attachments.data();
        createInfo.width = m_extent.width;
        createInfo.height = m_extent.height;
        createInfo.layers = 1;

        auto result = m_scene.engine().impl().device().createFramebufferUnique(createInfo);
        cascades[i].framebuffer = vulkan::checkMove(result, "stages.shadows", "Unable to create framebuffers.");
    }
}
