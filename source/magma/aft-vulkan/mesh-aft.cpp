#include "./mesh-aft.hpp"

#include <lava/magma/mesh.hpp>

#include "../vulkan/render-scenes/render-scene-impl.hpp"
#include "./material-aft.hpp"

using namespace lava::chamber;
using namespace lava::magma;

MeshAft::MeshAft(Mesh& fore, RenderScene::Impl& scene)
    : m_fore(fore)
    , m_scene(scene)
    , m_unlitVertexBufferHolder(m_scene.engine())
    , m_vertexBufferHolder(m_scene.engine())
    , m_indexBufferHolder(m_scene.engine())
{
}

void MeshAft::init()
{
    // Init UBO
    foreTransformChanged();
}

void MeshAft::update()
{
    if (m_vertexBufferDirty) {
        createVertexBuffer();
    }
}

void MeshAft::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset,
                     uint32_t materialDescriptorSetIndex) const
{
    if (m_fore.indices().empty()) return;

    // Bind the material
    // @todo Have this in a more clever render loop, and not called by this mesh
    // Fact is we shouldn't know about the correct descriptorSetIndex here
    if (m_fore.material() != nullptr) {
        m_fore.material()->aft().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
    }
    else {
        m_scene.fallbackMaterial().aft().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
    }

    // Add the vertex buffer
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, &m_vertexBufferHolder.buffer(), offsets);
    commandBuffer.bindIndexBuffer(m_indexBufferHolder.buffer(), 0, vk::IndexType::eUint16);

    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                pushConstantOffset, sizeof(MeshUbo), &m_ubo);

    // Draw
    commandBuffer.drawIndexed(m_fore.indices().size(), 1, 0, 0, 0);
}

void MeshAft::renderUnlit(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset) const
{
    if (m_fore.indices().empty()) return;

    // Add the vertex buffer
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, &m_unlitVertexBufferHolder.buffer(), offsets);
    commandBuffer.bindIndexBuffer(m_indexBufferHolder.buffer(), 0, vk::IndexType::eUint16);

    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                pushConstantOffset, sizeof(MeshUbo), &m_ubo);

    // Draw
    commandBuffer.drawIndexed(m_fore.indices().size(), 1, 0, 0, 0);
}

// ----- Fore

void MeshAft::foreTransformChanged()
{
    // Update UBO whenever the transform changed.
    auto transposeTransform = glm::transpose(m_fore.transform());
    m_ubo.transform0 = transposeTransform[0];
    m_ubo.transform1 = transposeTransform[1];
    m_ubo.transform2 = transposeTransform[2];
}

void MeshAft::foreIndicesChanged()
{
    createIndexBuffer();
}

void MeshAft::createVertexBuffer()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    // Unlit (just holding vertices' positions)
    vk::DeviceSize bufferSize = sizeof(UnlitVertex) * m_fore.unlitVertices().size();
    m_unlitVertexBufferHolder.create(vk::BufferUsageFlagBits::eVertexBuffer, bufferSize);
    m_unlitVertexBufferHolder.copy(m_fore.unlitVertices().data(), bufferSize);

    // Lit
    bufferSize = sizeof(Vertex) * m_fore.vertices().size();
    m_vertexBufferHolder.create(vk::BufferUsageFlagBits::eVertexBuffer, bufferSize);
    m_vertexBufferHolder.copy(m_fore.vertices().data(), bufferSize);

    m_vertexBufferDirty = false;
}

void MeshAft::createIndexBuffer()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    if (m_fore.indices().empty()) {
        logger.warning("magma.vulkan.mesh") << "No indices provided. The mesh will not be visible." << std::endl;
        return;
    }

    vk::DeviceSize bufferSize = sizeof(uint16_t) * m_fore.indices().size();

    m_indexBufferHolder.create(vk::BufferUsageFlagBits::eIndexBuffer, bufferSize);
    m_indexBufferHolder.copy(m_fore.indices().data(), bufferSize);
}
