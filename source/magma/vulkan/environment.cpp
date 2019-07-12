#include "./environment.hpp"

#include "../aft-vulkan/texture-aft.hpp"
#include "./render-engine-impl.hpp"
#include "./render-scenes/render-scene-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

Environment::Environment(RenderScene::Impl& scene)
    : m_scene(scene)
    , m_radianceStage(scene)
    , m_irradianceStage(scene)
    , m_radianceImageHolder(scene.engine(), "magma.vulkan.environment.radiance-image")
    , m_irradianceImageHolder(scene.engine(), "magma.vulkan.environment.irradiance-image")
{
}

Environment::~Environment()
{
    if (m_initialized) {
        m_scene.environmentDescriptorHolder().freeSet(m_descriptorSet);
        m_scene.environmentDescriptorHolder().freeSet(m_basicDescriptorSet);
    }
}

void Environment::init()
{
    m_initialized = true;

    createResources();

    m_radianceStage.init(EnvironmentPrefilteringStage::Algorithm::Radiance);
    m_irradianceStage.init(EnvironmentPrefilteringStage::Algorithm::Irradiance);

    updateBrdfLutBindings();
    updateBasicBindings();
    if (m_texture != nullptr) {
        computeRadiance();
        computeIrradiance();
        updateBindings();
    }
}

void Environment::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const
{
    // The non-prefiltered version has been set to plain transparent if there is no binded texture,
    // so we're using this one in that case.
    if (m_texture == nullptr) {
        renderBasic(commandBuffer, pipelineLayout, descriptorSetIndex);
        return;
    }

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
}

void Environment::renderBasic(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                              uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1,
                                     &m_basicDescriptorSet, 0, nullptr);
}

void Environment::set(Texture* texture)
{
    m_texture = texture;
    updateBasicBindings();

    m_radianceStage.update({ENVIRONMENT_RADIANCE_SIZE, ENVIRONMENT_RADIANCE_SIZE});
    m_irradianceStage.update({ENVIRONMENT_IRRADIANCE_SIZE, ENVIRONMENT_IRRADIANCE_SIZE});

    computeRadiance();
    computeIrradiance();
    updateBindings();
}

// ----- Internal

void Environment::computeRadiance()
{
    if (!m_initialized) return;

    // Temporary command buffer
    auto& device = m_scene.engine().device();
    auto& queue = m_scene.engine().graphicsQueue();
    auto& commandPool = m_scene.engine().commandPool();

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    device.allocateCommandBuffers(&allocInfo, &commandBuffer);

    // ----- Radiance

    for (auto mipLevel = 0u; mipLevel < ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT; ++mipLevel) {
        for (auto faceIndex = 0u; faceIndex < 6u; ++faceIndex) {
            // Record
            vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
            commandBuffer.begin(&beginInfo);

            m_radianceStage.render(commandBuffer, faceIndex, mipLevel);
            m_radianceStage.imageHolder().changeLayout(vk::ImageLayout::eTransferSrcOptimal, commandBuffer);

            commandBuffer.end();

            // Execute
            vk::SubmitInfo submitInfo;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            queue.submit(1, &submitInfo, nullptr);
            queue.waitIdle();

            // Copy image to the cube
            m_radianceImageHolder.copy(m_radianceStage.image(), faceIndex, mipLevel);
        }
    }

    device.freeCommandBuffers(commandPool, 1, &commandBuffer);

    updateBindings();
}

void Environment::computeIrradiance()
{
    if (!m_initialized) return;

    // Temporary command buffer
    auto& device = m_scene.engine().device();
    auto& queue = m_scene.engine().graphicsQueue();
    auto& commandPool = m_scene.engine().commandPool();

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    device.allocateCommandBuffers(&allocInfo, &commandBuffer);

    // ----- Irradiance

    for (auto faceIndex = 0u; faceIndex < 6u; ++faceIndex) {
        // Record
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        commandBuffer.begin(&beginInfo);

        m_irradianceStage.render(commandBuffer, faceIndex);
        m_irradianceStage.imageHolder().changeLayout(vk::ImageLayout::eTransferSrcOptimal, commandBuffer);

        commandBuffer.end();

        // Execute
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        queue.submit(1, &submitInfo, nullptr);
        queue.waitIdle();

        // Copy image to the cube
        m_irradianceImageHolder.copy(m_irradianceStage.image(), faceIndex);
    }

    device.freeCommandBuffers(commandPool, 1, &commandBuffer);

    updateBindings();
}

void Environment::createResources()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    logger.info("magma.vulkan.environment") << "Set to generate " << ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT
                                            << " mip levels for prefiltering." << std::endl;

    m_descriptorSet = m_scene.environmentDescriptorHolder().allocateSet("environment");
    m_basicDescriptorSet = m_scene.environmentDescriptorHolder().allocateSet("environment.non-prefiltered");

    // Radiance
    m_radianceImageHolder.create(vk::Format::eR16G16B16A16Sfloat, {ENVIRONMENT_RADIANCE_SIZE, ENVIRONMENT_RADIANCE_SIZE},
                                 vk::ImageAspectFlagBits::eColor, 6u, ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT);

    // Irradiance
    m_irradianceImageHolder.create(vk::Format::eR16G16B16A16Sfloat, {ENVIRONMENT_IRRADIANCE_SIZE, ENVIRONMENT_IRRADIANCE_SIZE},
                                   vk::ImageAspectFlagBits::eColor, 6u);

    // BRDF look-up texture
    m_brdfLutTexture = &m_scene.scene().make<Texture>();
    m_brdfLutTexture->loadFromFile("./data/textures/brdf-lut.png");
}

void Environment::updateBrdfLutBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto& engine = m_scene.engine();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    vk::ImageView imageView = m_brdfLutTexture->aft().imageView();

    vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, imageView, sampler, imageLayout, 2u);
    vulkan::updateDescriptorSet(engine.device(), m_basicDescriptorSet, imageView, sampler, imageLayout, 2u);
}

void Environment::updateBasicBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // Force transparent cube
    auto& engine = m_scene.engine();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    vk::ImageView imageView = engine.dummyCubeImageView();

    // Or use texture if provided!
    if (m_texture != nullptr) {
        imageView = m_texture->aft().imageView();
    }

    vulkan::updateDescriptorSet(engine.device(), m_basicDescriptorSet, imageView, sampler, imageLayout, 0u);
    vulkan::updateDescriptorSet(engine.device(), m_basicDescriptorSet, imageView, sampler, imageLayout, 1u);
}

void Environment::updateBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto& engine = m_scene.engine();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::ImageView imageView = m_radianceImageHolder.view();
    vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, imageView, sampler, imageLayout, 0u);

    imageView = m_irradianceImageHolder.view();
    vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, imageView, sampler, imageLayout, 1u);
}
