#include "./environment-prefiltering-stage.hpp"

#include "../../aft-vulkan/mesh-aft.hpp"
#include "../lights/i-light-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

EnvironmentPrefilteringStage::EnvironmentPrefilteringStage(RenderScene::Impl& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine())
    , m_pipelineHolder(m_scene.engine())
    , m_imageHolder(scene.engine(), "magma.vulkan.stages.environment-prefiltering-stage.image")
    , m_framebuffer(m_scene.engine().device())
{
}

void EnvironmentPrefilteringStage::init(Algorithm algorithm)
{
    m_algorithm = algorithm;

    logger.info("magma.vulkan.stages.environment-prefiltering-stage") << "Initializing." << std::endl;
    logger.log().tab(1);

    initPass();

    //----- Render pass

    m_renderPassHolder.add(m_pipelineHolder);
    m_renderPassHolder.init();

    logger.log().tab(-1);
}

void EnvironmentPrefilteringStage::update(vk::Extent2D extent)
{
    m_extent = extent;

    m_pipelineHolder.update(extent);

    createResources();
    createFramebuffers();
}

void EnvironmentPrefilteringStage::render(vk::CommandBuffer commandBuffer, uint8_t faceIndex)
{
    render(commandBuffer, faceIndex, 0u);
}

void EnvironmentPrefilteringStage::render(vk::CommandBuffer commandBuffer, uint8_t faceIndex, uint8_t mipLevel)
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    const auto& deviceHolder = m_scene.engine().deviceHolder();
    deviceHolder.debugBeginRegion(commandBuffer, "environment-prefiltering");

    //----- Prologue

    auto width = m_extent.width;
    auto height = m_extent.height;
    for (auto i = 0u; i < mipLevel; ++i) {
        width /= 2;
        height /= 2;
    }

    vk::Viewport viewport;
    viewport.width = width;
    viewport.height = height;
    commandBuffer.setViewport(0, 1, &viewport);

    // Set render pass
    std::array<vk::ClearValue, 1> clearValues;

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPassHolder.renderPass();
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Pass

    deviceHolder.debugBeginRegion(commandBuffer, "environment-prefiltering.pass");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelineHolder.pipeline());

    m_scene.environment().renderBasic(commandBuffer, m_pipelineHolder.pipelineLayout(), 0u);

    // The transform needed for each face
    std::vector<glm::mat4> matrices = {
        glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    };

    if (m_algorithm == Algorithm::Radiance) {
        m_radianceUbo.mvp = glm::perspective(math::PI / 2.f, 1.0f, 0.1f, (float)m_extent.width) * matrices[faceIndex];
        m_radianceUbo.roughness = static_cast<float>(mipLevel) / (ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT - 1);
        m_radianceUbo.samplesCount = SAMPLES_COUNT;
        commandBuffer.pushConstants(m_pipelineHolder.pipelineLayout(),
                                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                    sizeof(EnvironmentRadianceUbo), &m_radianceUbo);
    }
    else if (m_algorithm == Algorithm::Irradiance) {
        m_irradianceUbo.mvp = glm::perspective(math::PI / 2.f, 1.0f, 0.1f, (float)m_extent.width) * matrices[faceIndex];
        m_irradianceUbo.deltaPhi = (2.0f * math::PI) / 180.0f;
        m_irradianceUbo.deltaTheta = (0.5f * math::PI) / 64.0f;
        commandBuffer.pushConstants(m_pipelineHolder.pipelineLayout(),
                                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                    sizeof(EnvironmentIrradianceUbo), &m_irradianceUbo);
    }

    commandBuffer.draw(6, 1, 0, 0);

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugEndRegion(commandBuffer);
}

//----- Internal

void EnvironmentPrefilteringStage::initPass()
{
    std::string algorithm = "unknown";
    if (m_algorithm == Algorithm::Radiance)
        algorithm = "radiance";
    else if (m_algorithm == Algorithm::Irradiance)
        algorithm = "irradiance";

    //----- Shaders

    ShadersManager::ModuleOptions moduleOptions;

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/environment-prefiltering.vert", moduleOptions);
    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/environment-" + algorithm + ".frag", moduleOptions);
    m_pipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_pipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    //----- Descriptor set layouts

    // @note Ordering is important
    m_pipelineHolder.add(m_scene.environmentDescriptorHolder().setLayout());

    //----- Push constants

    if (m_algorithm == Algorithm::Radiance)
        m_pipelineHolder.addPushConstantRange(sizeof(EnvironmentRadianceUbo));
    else if (m_algorithm == Algorithm::Irradiance)
        m_pipelineHolder.addPushConstantRange(sizeof(EnvironmentIrradianceUbo));

    //----- Attachments

    vulkan::PipelineHolder::ColorAttachment colorAttachment;
    colorAttachment.format = vk::Format::eR16G16B16A16Sfloat;
    colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    m_pipelineHolder.add(colorAttachment);

    //----- Rasterization

    m_pipelineHolder.set(vk::CullModeFlagBits::eNone);
    m_pipelineHolder.dynamicViewportEnabled(true);
}

void EnvironmentPrefilteringStage::createResources()
{
    auto format = vk::Format::eR16G16B16A16Sfloat;
    m_imageHolder.create(format, m_extent, vk::ImageAspectFlagBits::eColor, 6u);
}

void EnvironmentPrefilteringStage::createFramebuffers()
{
    // Framebuffer
    std::array<vk::ImageView, 1> attachments = {m_imageHolder.view()};

    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPassHolder.renderPass();
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_scene.engine().device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.environment-prefiltering-stage") << "Failed to create framebuffers." << std::endl;
    }
}
