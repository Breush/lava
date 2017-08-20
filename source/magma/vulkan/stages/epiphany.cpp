#include "./epiphany.hpp"

#include <lava/chamber/logger.hpp>

#include "../helpers/shader.hpp"
#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../vertex.hpp"

namespace {
    struct CameraUbo {
        glm::mat4 invertedView;
        glm::mat4 invertedProjection;
        glm::vec4 wPosition;
    };

    struct LightUbo {
        glm::vec4 wPosition;
        float radius;
    };

    struct Vertex {
        glm::vec2 position;
    };
}

using namespace lava::magma;
using namespace lava::chamber;

Epiphany::Epiphany(RenderScene::Impl& scene)
    : RenderStage(scene.engine())
    , m_scene(scene)
    , m_vertexShaderModule{m_engine.device()}
    , m_fragmentShaderModule{m_engine.device()}
    , m_imageHolder{m_engine}
    , m_uboHolder(m_engine)
    , m_descriptorHolder(m_engine)
    , m_framebuffer{m_engine.device()}
{
}

//----- RenderStage

void Epiphany::stageInit()
{
    logger.info("magma.vulkan.stages.epiphany") << "Initializing." << std::endl;
    logger.log().tab(1);

    //----- Shaders

    auto vertexShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/epiphany.vert");
    m_vertexShaderModule = vulkan::createShaderModule(m_engine.device(), vertexShaderCode);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, m_vertexShaderModule, "main"});

    auto fragmentShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/epiphany-phong.frag");
    m_fragmentShaderModule = vulkan::createShaderModule(m_engine.device(), fragmentShaderCode);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, m_fragmentShaderModule, "main"});

    //----- Descriptors

    // UBO: camera, light
    // CIS: normal, albedo, orm, depth
    m_descriptorHolder.init(2, 4, 1, vk::ShaderStageFlagBits::eFragment);
    add(m_descriptorHolder.setLayout());

    m_descriptorSet = m_descriptorHolder.allocateSet();

    //----- Uniform buffers

    m_uboHolder.init(m_descriptorSet, {sizeof(CameraUbo), sizeof(LightUbo)});

    //----- Attachments

    ColorAttachment targetColorAttachment;
    targetColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    targetColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    add(targetColorAttachment);

    logger.log().tab(-1);
}

void Epiphany::stageUpdate()
{
    logger.info("magma.vulkan.stages.epiphany") << "Updating." << std::endl;
    logger.log().tab(1);

    createResources();
    createFramebuffers();

    logger.log().tab(-1);
}

void Epiphany::stageRender(const vk::CommandBuffer& commandBuffer)
{
    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 1> clearValues;
    // clearValues[0] does not matter

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    // Light linked list
    fillLll();

    // Update UBOs
    updateUbos();

    //----- Render

    // Bind pipeline
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    // Draw
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
    commandBuffer.draw(6, 1, 0, 0);

    //----- Epilogue

    commandBuffer.endRenderPass();
}

//----- Internal

void Epiphany::createResources()
{
    // Target
    auto format = vk::Format::eR8G8B8A8Unorm;
    m_imageHolder.create(format, m_extent, vk::ImageAspectFlagBits::eColor);
}

void Epiphany::createFramebuffers()
{
    // Framebuffer
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &m_imageHolder.view();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_engine.device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.g-buffer") << "Failed to create framebuffers." << std::endl;
    }
}

// @todo Have vulkan::DescriptorCombinedImageSampler
void Epiphany::normalImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 2;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::albedoImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 3;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::ormImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 4;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::depthImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    vk::DescriptorImageInfo imageInfo;
    // @note Correspond to the final layout specified at previous pass
    imageInfo.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 5;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::updateUbos()
{
    //----- Camera UBO

    if (m_scene.cameras().size() > 0) {
        const auto& camera = m_scene.camera(0);

        CameraUbo ubo;
        ubo.invertedView = glm::inverse(camera.viewTransform());
        ubo.invertedProjection = glm::inverse(camera.projectionTransform());
        ubo.wPosition = glm::vec4(camera.position(), 1.f);

        m_uboHolder.copy(0, ubo);
    }

    //----- Light UBO

    if (m_scene.pointLights().size() > 0) {
        const auto& pointLight = m_scene.pointLight(0);

        LightUbo ubo;
        ubo.wPosition = glm::vec4(pointLight.position(), 1.f);
        ubo.radius = pointLight.radius();

        m_uboHolder.copy(1, ubo);
    }
}

void Epiphany::fillLll()
{
    // @todo Render a sphere for the light
}
