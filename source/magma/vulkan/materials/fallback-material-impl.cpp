#include "./fallback-material-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <vulkan/vulkan.h>

#include "../helpers/descriptor.hpp"
#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../ubos.hpp"

#include <cstring>

using namespace lava::magma;
using namespace lava::chamber;

FallbackMaterial::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
{
}

FallbackMaterial::Impl::~Impl()
{
}

//----- IMaterial

void FallbackMaterial::Impl::init()
{
    m_descriptorSet = m_scene.materialDescriptorHolder().allocateSet(true);
    m_uboHolder.init(m_descriptorSet, m_scene.materialDescriptorHolder().uniformBufferBindingOffset(), {sizeof(uint32_t)});

    uint32_t id = FallbackMaterial::materialId();
    m_uboHolder.copy(0, id);
}

void FallbackMaterial::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                                    uint32_t descriptorSetIndex)
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
}
