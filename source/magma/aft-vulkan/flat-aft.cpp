#include "./flat-aft.hpp"

#include <lava/magma/flat.hpp>
#include <lava/magma/material.hpp>
#include <lava/magma/scene.hpp>

#include "./material-aft.hpp"
#include "./scene-aft.hpp"

using namespace lava::chamber;
using namespace lava::magma;

FlatAft::FlatAft(Flat& fore, Scene& scene)
    : m_fore(fore)
    , m_scene(scene)
    , m_vertexBufferHolder(m_scene.engine().impl(), "flat.vertex")
    , m_indexBufferHolder(m_scene.engine().impl(), "flat.index")
{
}

void FlatAft::update()
{
    if (m_vertexBufferDirty) {
        createVertexBuffer();
    }
}

void FlatAft::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset,
                     uint32_t materialDescriptorSetIndex) const
{
    if (m_fore.indices().empty()) return;

    // Bind the material @todo :CleverMaterialBinding
    if (m_fore.material() != nullptr) {
        m_fore.material()->aft().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
    }
    else {
        m_scene.fallbackMaterial()->aft().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
    }

    // Add the vertex buffer
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, &m_vertexBufferHolder.buffer(), offsets);
    commandBuffer.bindIndexBuffer(m_indexBufferHolder.buffer(), 0, vk::IndexType::eUint16);

    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                pushConstantOffset, sizeof(FlatUbo), &m_fore.ubo());

    // Draw
    commandBuffer.drawIndexed(m_fore.indices().size(), 1, 0, 0, 0);
}

// ----- Fore

void FlatAft::foreIndicesChanged()
{
    createIndexBuffer();
}

void FlatAft::createVertexBuffer()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    // @todo :SlowMeshVertexUpdate By saying OnDemandStaging, we are guessing the data will not evolve too much over time.
    vk::DeviceSize bufferSize = sizeof(FlatVertex) * m_fore.vertices().size();
    m_vertexBufferHolder.create(vulkan::BufferKind::ShaderVertex, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_vertexBufferHolder.copy(m_fore.vertices().data(), bufferSize);

    m_vertexBufferDirty = false;
}

void FlatAft::createIndexBuffer()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    if (m_fore.indices().empty()) {
        logger.warning("magma.vulkan.flat") << "No indices provided. The flat will not be visible." << std::endl;
        return;
    }

    vk::DeviceSize bufferSize = sizeof(uint16_t) * m_fore.indices().size();

    m_indexBufferHolder.create(vulkan::BufferKind::ShaderIndex, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_indexBufferHolder.copy(m_fore.indices().data(), bufferSize);
}
