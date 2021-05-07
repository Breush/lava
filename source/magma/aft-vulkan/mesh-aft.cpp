#include "./mesh-aft.hpp"

#include <lava/magma/material.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/scene.hpp>

#include "../vulkan/render-engine-impl.hpp"
#include "./material-aft.hpp"
#include "./scene-aft.hpp"

using namespace lava::chamber;
using namespace lava::magma;

MeshAft::MeshAft(Mesh& fore, Scene& scene)
    : m_fore(fore)
    , m_scene(scene)
    , m_blasHolder(m_scene.engine().impl())
    , m_blasTransformBufferHolder(m_scene.engine().impl(), "mesh.blas-transform")
    , m_unlitVertexBufferHolder(m_scene.engine().impl(), "mesh.unlit-vertex")
    , m_vertexBufferHolder(m_scene.engine().impl(), "mesh.vertex")
    , m_instanceBufferHolder(m_scene.engine().impl(), "mesh.instance")
    , m_indexBufferHolder(m_scene.engine().impl(), "mesh.index")
{
}

void MeshAft::update()
{
    if (m_vertexBufferDirty) {
        createVertexBuffers();
    }

    if (m_instanceBufferDirty) {
        createInstanceBuffer();

        // @fixme Have own dirty flag
        createBlas();
    }
}

void MeshAft::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                     uint32_t materialDescriptorSetIndex) const
{
    if (!m_fore.enabled() || m_fore.indices().empty()) return;

    // Bind the material
    // @todo :CleverMaterialBinding Have this in a more clever render loop, and not called by this mesh
    // Fact is we shouldn't know about the correct descriptorSetIndex here
    if (m_fore.material() != nullptr) {
        m_fore.material()->aft().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
    }
    else {
        m_scene.fallbackMaterial()->aft().render(commandBuffer, pipelineLayout, materialDescriptorSetIndex);
    }

    // Add the vertex buffer
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, &m_vertexBufferHolder.buffer(), offsets);
    commandBuffer.bindVertexBuffers(1, 1, &m_instanceBufferHolder.buffer(), offsets);
    commandBuffer.bindIndexBuffer(m_indexBufferHolder.buffer(), 0, vk::IndexType::eUint16);

    // Draw
    commandBuffer.drawIndexed(m_fore.indices().size(), m_fore.instancesCount(), 0, 0, 0);
}

void MeshAft::renderUnlit(vk::CommandBuffer commandBuffer) const
{
    if (!m_fore.enabled() || m_fore.indices().empty()) return;

    // Add the vertex buffer
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, &m_unlitVertexBufferHolder.buffer(), offsets);
    commandBuffer.bindVertexBuffers(1, 1, &m_instanceBufferHolder.buffer(), offsets);
    commandBuffer.bindIndexBuffer(m_indexBufferHolder.buffer(), 0, vk::IndexType::eUint16);

    // Draw
    commandBuffer.drawIndexed(m_fore.indices().size(), m_fore.instancesCount(), 0, 0, 0);
}

// ----- Fore

void MeshAft::foreIndicesChanged()
{
    createIndexBuffer();
}

void MeshAft::createVertexBuffers()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    // @todo :SlowMeshVertexUpdate
    // Unlit (just holding vertices' positions)
    vk::DeviceSize bufferSize = sizeof(UnlitVertex) * m_fore.unlitVertices().size();
    // m_unlitVertexBufferHolder.create(vulkan::BufferKind::ShaderVertex, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_unlitVertexBufferHolder.create(vulkan::BufferKind::AccelerationStructureInput, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_unlitVertexBufferHolder.copy(m_fore.unlitVertices().data(), bufferSize);

    // Lit
    bufferSize = sizeof(Vertex) * m_fore.vertices().size();
    // m_vertexBufferHolder.create(vulkan::BufferKind::ShaderVertex, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_vertexBufferHolder.create(vulkan::BufferKind::AccelerationStructureInput, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_vertexBufferHolder.copy(m_fore.vertices().data(), bufferSize);

    m_vertexBufferDirty = false;
}

void MeshAft::createInstanceBuffer()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);
    m_scene.engine().impl().device().waitIdle();

    vk::DeviceSize bufferSize = sizeof(MeshUbo) * m_fore.instancesCount();
    // m_instanceBufferHolder.create(vulkan::BufferKind::ShaderVertex, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_instanceBufferHolder.create(vulkan::BufferKind::AccelerationStructureInput, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_instanceBufferHolder.copy(m_fore.ubos().data(), bufferSize);

    m_instanceBufferDirty = false;
}

void MeshAft::createIndexBuffer()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    if (m_fore.indices().empty()) {
        logger.warning("magma.vulkan.mesh") << "No indices provided. The mesh will not be visible." << std::endl;
        return;
    }

    vk::DeviceSize bufferSize = sizeof(uint16_t) * m_fore.indices().size();

    // m_indexBufferHolder.create(vulkan::BufferKind::ShaderIndex, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_indexBufferHolder.create(vulkan::BufferKind::AccelerationStructureInput, vulkan::BufferCpuIo::OnDemandStaging, bufferSize);
    m_indexBufferHolder.copy(m_fore.indices().data(), bufferSize);
}

void MeshAft::createBlas()
{
    // @fixme The transform needs to be updated each time?
    // Better think about that!

    // @fixme Well, whatever, this should be Identity here,
    // It's the instance one that should hold the transform.
    auto transform = glm::transpose(m_fore.transform());
    vk::TransformMatrixKHR transformMatrix;
    transformMatrix.matrix[0][0] = transform[0][0];
    transformMatrix.matrix[0][1] = transform[0][1];
    transformMatrix.matrix[0][2] = transform[0][2];
    transformMatrix.matrix[0][3] = transform[0][3];
    transformMatrix.matrix[1][0] = transform[1][0];
    transformMatrix.matrix[1][1] = transform[1][1];
    transformMatrix.matrix[1][2] = transform[1][2];
    transformMatrix.matrix[1][3] = transform[1][3];
    transformMatrix.matrix[2][0] = transform[2][0];
    transformMatrix.matrix[2][1] = transform[2][1];
    transformMatrix.matrix[2][2] = transform[2][2];
    transformMatrix.matrix[2][3] = transform[2][3];

    // Create buffers
    m_blasTransformBufferHolder.create(vulkan::BufferKind::AccelerationStructureInput, vulkan::BufferCpuIo::OnDemandStaging, sizeof(vk::TransformMatrixKHR));
    m_blasTransformBufferHolder.copy(&transformMatrix, sizeof(vk::TransformMatrixKHR));

    // Build AS
    // @fixme HEY, I've just noticed that we do not need unlit vertex buffer for shadows
    // thanks to the stride. Probably there is the very same thing with graphics pipeline.
    vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
    triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
    triangles.vertexData = m_unlitVertexBufferHolder.deviceAddress(); // @fixme FORCED?
    triangles.vertexStride = sizeof(UnlitVertex);
    triangles.maxVertex = m_fore.verticesCount();
    triangles.indexType = vk::IndexType::eUint16;
    triangles.indexData = m_indexBufferHolder.deviceAddress();
    triangles.transformData = m_blasTransformBufferHolder.deviceAddress();

    vk::AccelerationStructureGeometryKHR accelerationStructureGeometry;
    accelerationStructureGeometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
    accelerationStructureGeometry.geometryType = vk::GeometryTypeKHR::eTriangles;
    accelerationStructureGeometry.geometry.triangles = triangles;

    m_blasHolder.primitiveCount(m_fore.indices().size() / 3u);
    m_blasHolder.create(vk::AccelerationStructureTypeKHR::eBottomLevel, accelerationStructureGeometry);
}
