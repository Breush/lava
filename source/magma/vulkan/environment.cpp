#include "./environment.hpp"

#include <lava/magma/scene.hpp>
#include <lava/magma/texture.hpp>

#include "../aft-vulkan/scene-aft.hpp"
#include "../aft-vulkan/texture-aft.hpp"
#include "./render-engine-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

Environment::Environment(Scene& scene, RenderEngine& engine)
    : m_scene(scene)
    , m_radianceStage(scene, engine)
    , m_irradianceStage(scene, engine)
    , m_radianceImageHolder(engine.impl(), "environment.radiance")
    , m_irradianceImageHolder(engine.impl(), "environment.irradiance")
{
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

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet.get(), 0,
                                     nullptr);
}

void Environment::renderBasic(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                              uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1,
                                     &m_basicDescriptorSet.get(), 0, nullptr);
}

void Environment::set(const TexturePtr& texture)
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
    auto& engine = m_scene.engine().impl();
    auto& device = engine.device();
    auto& queue = engine.graphicsQueue();
    auto commandPool = engine.commandPool();

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    auto result = device.allocateCommandBuffersUnique(allocInfo);
    auto commandBuffer = std::move(vulkan::checkMove(result, "environment", "Unable to create command buffer.")[0]);

    // ----- Radiance

    for (auto mipLevel = 0u; mipLevel < ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT; ++mipLevel) {
        for (auto faceIndex = 0u; faceIndex < 6u; ++faceIndex) {
            // Record
            vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
            commandBuffer->begin(&beginInfo);

            m_radianceStage.render(commandBuffer.get(), faceIndex, mipLevel);
            m_radianceStage.imageHolder().changeLayout(vk::ImageLayout::eTransferSrcOptimal, commandBuffer.get());

            commandBuffer->end();

            // Execute
            vk::SubmitInfo submitInfo;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer.get();
            queue.submit(1, &submitInfo, nullptr);
            queue.waitIdle();

            // Copy image to the cube
            m_radianceImageHolder.copy(m_radianceStage.image(), faceIndex, mipLevel);
        }
    }

    updateBindings();
}

void Environment::computeIrradiance()
{
    if (!m_initialized) return;

    // Temporary command buffer
    auto& engine = m_scene.engine().impl();
    auto& device = engine.device();
    auto& queue = engine.graphicsQueue();
    auto commandPool = engine.commandPool();

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    auto result = device.allocateCommandBuffersUnique(allocInfo);
    auto commandBuffer = std::move(vulkan::checkMove(result, "environment", "Unable to create command buffer.")[0]);

    // ----- Irradiance

    for (auto faceIndex = 0u; faceIndex < 6u; ++faceIndex) {
        // Record
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        commandBuffer->begin(&beginInfo);

        m_irradianceStage.render(commandBuffer.get(), faceIndex);
        m_irradianceStage.imageHolder().changeLayout(vk::ImageLayout::eTransferSrcOptimal, commandBuffer.get());

        commandBuffer->end();

        // Execute
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer.get();
        queue.submit(1, &submitInfo, nullptr);
        queue.waitIdle();

        // Copy image to the cube
        m_irradianceImageHolder.copy(m_irradianceStage.image(), faceIndex);
    }

    updateBindings();
}

void Environment::createResources()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    logger.info("magma.vulkan.environment") << "Set to generate " << ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT
                                            << " mip levels for prefiltering." << std::endl;

    m_descriptorSet = m_scene.aft().environmentDescriptorHolder().allocateSet("environment");
    m_basicDescriptorSet = m_scene.aft().environmentDescriptorHolder().allocateSet("environment.non-prefiltered");

    // Radiance
    m_radianceImageHolder.create(vulkan::ImageKind::Texture, vk::Format::eR8G8B8A8Unorm,
                                 {ENVIRONMENT_RADIANCE_SIZE, ENVIRONMENT_RADIANCE_SIZE},
                                 6u, ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT);

    // Irradiance
    m_irradianceImageHolder.create(vulkan::ImageKind::Texture, vk::Format::eR8G8B8A8Unorm,
                                   {ENVIRONMENT_IRRADIANCE_SIZE, ENVIRONMENT_IRRADIANCE_SIZE}, 6u);

    // BRDF look-up texture
    m_brdfLutTexture = m_scene.engine().makeTexture();
    m_brdfLutTexture->loadFromFile("./data/textures/brdf-lut.png");
}

void Environment::updateBrdfLutBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto& engine = m_scene.engine().impl();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    vk::ImageView imageView = m_brdfLutTexture->aft().imageView();

    vulkan::updateDescriptorSet(engine.device(), m_descriptorSet.get(), imageView, sampler, imageLayout, 2u);
    vulkan::updateDescriptorSet(engine.device(), m_basicDescriptorSet.get(), imageView, sampler, imageLayout, 2u);
}

void Environment::updateBasicBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // Force transparent cube
    auto& engine = m_scene.engine().impl();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    vk::ImageView imageView = engine.dummyCubeImageView();

    // Or use texture if provided!
    if (m_texture != nullptr) {
        imageView = m_texture->aft().imageView();
    }

    vulkan::updateDescriptorSet(engine.device(), m_basicDescriptorSet.get(), imageView, sampler, imageLayout, 0u);
    vulkan::updateDescriptorSet(engine.device(), m_basicDescriptorSet.get(), imageView, sampler, imageLayout, 1u);
}

void Environment::updateBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto& engine = m_scene.engine().impl();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::ImageView imageView = m_radianceImageHolder.view();
    vulkan::updateDescriptorSet(engine.device(), m_descriptorSet.get(), imageView, sampler, imageLayout, 0u);

    imageView = m_irradianceImageHolder.view();
    vulkan::updateDescriptorSet(engine.device(), m_descriptorSet.get(), imageView, sampler, imageLayout, 1u);
}
