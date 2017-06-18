#include "./mrr-material-impl.hpp"

#include "./render-engine-impl.hpp"

#include <cstring>

namespace {
    void cleanAttribute(lava::MrrMaterial::Impl::Attribute& attribute)
    {
        if (attribute.type == lava::MrrMaterial::Impl::Attribute::Type::TEXTURE) {
            delete[] attribute.texture.pixels;
        }
        attribute.type = lava::MrrMaterial::Impl::Attribute::Type::NONE;
    }
}

using namespace lava;

MrrMaterial::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
{
    m_baseColor.type = Attribute::Type::NONE;
}

MrrMaterial::Impl::~Impl()
{
    cleanAttribute(m_baseColor);
}

// @todo This should be a reference to a texture, so that it can be shared between materials
void MrrMaterial::Impl::baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    cleanAttribute(m_baseColor);

    m_baseColor.type = Attribute::Type::TEXTURE;
    m_baseColor.texture.width = width;
    m_baseColor.texture.height = height;
    m_baseColor.texture.channels = channels;
    m_baseColor.texture.pixels = new uint8_t[width * height];
    memcpy(m_baseColor.texture.pixels, pixels.data(), width * height);
}

void MrrMaterial::Impl::addCommands(VkCommandBuffer commandBuffer)
{
    // @todo Bind whatever is needed!
    /*vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_engine.pipelineLayout(), 0, 1, &m_descriptorSet,
       0,
                            nullptr);*/
}
