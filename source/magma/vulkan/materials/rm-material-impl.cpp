#include "./rm-material-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <vulkan/vulkan.h>

#include "../helpers/descriptor.hpp"
#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../ubos.hpp"

#include <cstring>

using namespace lava;

namespace {
    void cleanAttribute(magma::RmMaterial::Impl::Attribute& attribute)
    {
        if (attribute.type == magma::RmMaterial::Impl::Attribute::Type::TEXTURE) {
            delete[] attribute.texture.pixels;
        }
        attribute.type = magma::RmMaterial::Impl::Attribute::Type::NONE;
    }

    void setupTexture(magma::RmMaterial::Impl::Attribute::Texture& texture, const std::vector<uint8_t>& pixels, uint32_t width,
                      uint32_t height, uint8_t channels)
    {
        texture.width = width;
        texture.height = height;
        texture.channels = channels;

        if (pixels.size() != width * height * channels) {
            chamber::logger.error("magma.vulkan.rm-material")
                << "Image dimension for texture does not match provided data length."
                << " Data: " << pixels.size() << " Dimensions: " << width << "x" << height << " ("
                << static_cast<uint32_t>(channels) << ")" << std::endl;
        }

        texture.pixels = new uint8_t[pixels.size()];
        memcpy(texture.pixels, pixels.data(), pixels.size());
    }
}

using namespace lava::magma;
using namespace lava::chamber;

RmMaterial::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
    , m_normalImageHolder(m_scene.engine())
    , m_albedoImageHolder(m_scene.engine())
    , m_ormImageHolder(m_scene.engine())
{
    m_albedo.type = Attribute::Type::NONE;
    m_normal.type = Attribute::Type::NONE;
    m_orm.type = Attribute::Type::NONE;
}

RmMaterial::Impl::~Impl()
{
    cleanAttribute(m_albedo);
    cleanAttribute(m_normal);
    cleanAttribute(m_orm);
}

void RmMaterial::Impl::init()
{
    m_descriptorSet = m_scene.materialDescriptorHolder().allocateSet();
    m_uboHolder.init(m_descriptorSet, {sizeof(vulkan::MaterialUbo)});

    m_initialized = true;
    updateBindings();
}

void RmMaterial::Impl::roughness(float factor)
{
    m_roughnessFactor = factor;
    updateBindings();
}

void RmMaterial::Impl::metallic(float factor)
{
    m_metallicFactor = factor;
    updateBindings();
}

void RmMaterial::Impl::normal(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    cleanAttribute(m_normal);

    m_normal.type = Attribute::Type::TEXTURE;
    setupTexture(m_normal.texture, pixels, width, height, channels);
    m_normalImageHolder.setup(pixels, width, height, channels);
    updateBindings();
}

// @todo This should be a reference to a texture, so that it can be shared between materials
void RmMaterial::Impl::baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    cleanAttribute(m_albedo);

    m_albedo.type = Attribute::Type::TEXTURE;
    setupTexture(m_albedo.texture, pixels, width, height, channels);
    m_albedoImageHolder.setup(pixels, width, height, channels);
    updateBindings();
}

void RmMaterial::Impl::metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                                              uint8_t channels)
{
    cleanAttribute(m_orm);

    m_orm.type = Attribute::Type::TEXTURE;
    setupTexture(m_orm.texture, pixels, width, height, channels);
    m_ormImageHolder.setup(pixels, width, height, channels);
    updateBindings();
}

//----- IMaterial

void RmMaterial::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex)
{
    // @note This presuppose that the correct shader is binded

    // Bind with the material descriptor set
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
}

//----- Private

void RmMaterial::Impl::updateBindings()
{
    if (!m_initialized) return;

    // MaterialUbo
    vulkan::MaterialUbo ubo;
    ubo.roughnessFactor = m_roughnessFactor;
    ubo.metallicFactor = m_metallicFactor;
    m_uboHolder.copy(0, ubo);

    // Samplers
    const auto& engine = m_scene.engine();
    const auto& sampler = engine.dummySampler();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    const auto normalImageView =
        (m_normal.type == Attribute::Type::TEXTURE) ? m_normalImageHolder.view() : engine.dummyNormalImageView();
    vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, normalImageView, sampler, imageLayout, 1u);

    const auto albedoImageView =
        (m_albedo.type == Attribute::Type::TEXTURE) ? m_albedoImageHolder.view() : engine.dummyImageView();
    vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, albedoImageView, sampler, imageLayout, 2u);

    const auto ormImageView = (m_orm.type == Attribute::Type::TEXTURE) ? m_ormImageHolder.view() : engine.dummyImageView();
    vulkan::updateDescriptorSet(engine.device(), m_descriptorSet, ormImageView, sampler, imageLayout, 3u);
}
