#include "./ray-tracer-stage.hpp"

#include <lava/magma/camera.hpp>

#include "../../aft-vulkan/camera-aft.hpp"
#include "../../aft-vulkan/mesh-aft.hpp"
#include "../../aft-vulkan/scene-aft.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

RayTracerStage::RayTracerStage(Scene& scene)
    : m_scene(scene)
    , m_pipelineHolder(m_scene.engine().impl())
    , m_finalImageHolder(m_scene.engine().impl(), "stages.ray-tracer.final")
    , topLevelAS(m_scene.engine().impl())
    , indicesBuffer(m_scene.engine().impl())
    , verticesBuffer(m_scene.engine().impl())
    , descriptorHolder(m_scene.engine().impl())
    , raygenShaderBindingTable(m_scene.engine().impl())
    , missShaderBindingTable(m_scene.engine().impl())
    , hitShaderBindingTable(m_scene.engine().impl())
{
}

void RayTracerStage::init(const Camera& camera)
{
    m_camera = &camera;

    logger.info("magma.vulkan.stages.ray-tracer") << "Initializing." << std::endl;
    logger.log().tab(1);

    updatePassShaders(true);
    initPass();

    logger.log().tab(-1);
}

void RayTracerStage::rebuild()
{
    if (m_rebuildPipelines) {
        m_pipelineHolder.updateRaytracing();
        m_rebuildPipelines = false;
    }

    if (m_rebuildResources) {
        createResources();
        m_rebuildResources = false;
    }
}

void RayTracerStage::record(vk::CommandBuffer commandBuffer, uint32_t frameId)
{
    if (frameId != 0) return; // @fixme!!!

    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    const auto& deviceHolder = m_scene.engine().impl().deviceHolder();
    deviceHolder.debugBeginRegion(commandBuffer, "ray-tracer");

    //----- Pass

    deviceHolder.debugBeginRegion(commandBuffer, "ray-tracer.pass");

    // Draw
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineHolder.pipeline());

    // Set the camera
    m_camera->aft().render(commandBuffer, m_pipelineHolder.kind(), m_pipelineHolder.pipelineLayout(), CAMERA_PUSH_CONSTANT_OFFSET);

    // @fixme Have AccelerationStructure::render()?
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineHolder.pipelineLayout(), 0, 1, &descriptorSet.get(), 0, 0);

    commandBuffer.traceRaysKHR(&raygenShaderSbtEntry, &missShaderSbtEntry, &hitShaderSbtEntry, &callableShaderSbtEntry, m_extent.width, m_extent.height, 1);

    deviceHolder.debugEndRegion(commandBuffer);
}

void RayTracerStage::extent(const vk::Extent2D& extent)
{
    if (m_extent == extent) return;
    m_extent = extent;
    m_rebuildPipelines = true;
    m_rebuildResources = true;
}

RenderImage RayTracerStage::renderImage() const
{
    auto cameraId = m_scene.aft().cameraId(*m_camera);

    return m_finalImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA + cameraId);
}

void RayTracerStage::changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    m_finalImageHolder.changeLayout(imageLayout, commandBuffer);
}

//----- Internal

void RayTracerStage::initPass()
{
    //----- Descriptor set layouts

    descriptorHolder.setSizes(vulkan::DescriptorKind::AccelerationStructure, { 1 });
    descriptorHolder.setSizes(vulkan::DescriptorKind::StorageImage, { 1 });
    // @fixme Have two descriptors?
    // descriptorHolder.setSizes(vulkan::DescriptorKind::StorageBuffer, { 1, 1 }); // Indices, Vertices
    descriptorHolder.setSizes(vulkan::DescriptorKind::StorageBuffer, { 1024, 1024, 1024 }); // @fixme 1024 meshes only? Indices, Vertices
    descriptorHolder.init(1u, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR);
    descriptorSet = descriptorHolder.allocateSet("rt");

    m_pipelineHolder.add(descriptorHolder.setLayout());

    //----- Push constants

    m_pipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    m_pipelineHolder.init(PipelineKind::RayTracing, 0u);

    //----- Shader groups @fixme Use updateShadersPass!

    auto& shadersManager = m_scene.engine().impl().shadersManager();
    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["USE_CAMERA_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["USE_FLAT_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["USE_SHADOW_MAP_PUSH_CONSTANT"] = '0';

    // Ray generation group
    {
        auto shaderModule = shadersManager.module("./data/shaders/raygen.rgen", moduleOptions);
        m_pipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eRaygenKHR, shaderModule, "main"});

        vk::RayTracingShaderGroupCreateInfoKHR shaderGroup;
        shaderGroup.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
        shaderGroup.generalShader = 0u;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        m_pipelineHolder.add(shaderGroup);
    }

    // Miss group
    {
        auto shaderModule = shadersManager.module("./data/shaders/miss.rmiss");
        m_pipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eMissKHR, shaderModule, "main"});

        vk::RayTracingShaderGroupCreateInfoKHR shaderGroup;
        shaderGroup.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
        shaderGroup.generalShader = 1u;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        m_pipelineHolder.add(shaderGroup);
    }

    // Closest hit group
    {
        auto shaderModule = shadersManager.module("./data/shaders/closesthit.rchit");
        m_pipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eClosestHitKHR, shaderModule, "main"});

        vk::RayTracingShaderGroupCreateInfoKHR shaderGroup;
        shaderGroup.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
        shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.closestHitShader = 2u;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        m_pipelineHolder.add(shaderGroup);
    }

    // @note We need to do the update before the following thing

    if (m_rebuildPipelines) {
        m_pipelineHolder.updateRaytracing();
        m_rebuildPipelines = false;
    }

    // ------ @fixme createShaderBindingTable


    const auto& rtPipelineProperties = m_scene.engine().impl().deviceHolder().rtPipelineProperties();
    const uint32_t handleSize = rtPipelineProperties.shaderGroupHandleSize;
    const uint32_t handleSizeAligned = alignedSize(rtPipelineProperties.shaderGroupHandleSize, rtPipelineProperties.shaderGroupHandleAlignment);

    {
        const uint32_t groupCount = static_cast<uint32_t>(m_pipelineHolder.shaderGroups().size());
        const uint32_t sbtSize = groupCount * handleSizeAligned;

        std::vector<uint8_t> shaderHandleStorage(sbtSize);
        m_scene.engine().impl().device().getRayTracingShaderGroupHandlesKHR(m_pipelineHolder.pipeline(), 0, groupCount, sbtSize, shaderHandleStorage.data());

        raygenShaderBindingTable.create(vulkan::BufferKind::ShaderBindingTable, vulkan::BufferCpuIo::Direct, handleSize);
        missShaderBindingTable.create(vulkan::BufferKind::ShaderBindingTable, vulkan::BufferCpuIo::Direct, handleSize);
        hitShaderBindingTable.create(vulkan::BufferKind::ShaderBindingTable, vulkan::BufferCpuIo::Direct, handleSize);

        raygenShaderBindingTable.copy(shaderHandleStorage.data(), handleSize);
        missShaderBindingTable.copy(shaderHandleStorage.data() + handleSizeAligned, handleSize);
        hitShaderBindingTable.copy(shaderHandleStorage.data() + 2 * handleSizeAligned, handleSize);

        raygenShaderSbtEntry.deviceAddress = raygenShaderBindingTable.deviceAddress();
        raygenShaderSbtEntry.stride = handleSizeAligned;
        raygenShaderSbtEntry.size = handleSizeAligned;

        missShaderSbtEntry.deviceAddress = missShaderBindingTable.deviceAddress();
        missShaderSbtEntry.stride = handleSizeAligned;
        missShaderSbtEntry.size = handleSizeAligned;

        hitShaderSbtEntry.deviceAddress = hitShaderBindingTable.deviceAddress();
        hitShaderSbtEntry.stride = handleSizeAligned;
        hitShaderSbtEntry.size = handleSizeAligned;
    }
}

void RayTracerStage::createAS()
{
    // Setup identity transform matrix
    vk::TransformMatrixKHR transformMatrix = VkTransformMatrixKHR{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    };

    // ------- @fixme createTopLevelAccelerationStructure
    // @fixme That TLAS should be in SCENE and rebuild when needed

    // @fixme YEAP...
    for (auto mesh : m_scene.meshes()) {
        mesh->aft().update();
    }

    {
        std::vector<vk::AccelerationStructureInstanceKHR> instances;
        instances.reserve(m_scene.meshes().size());

        for (auto i = 0u; i < m_scene.meshes().size(); ++i) {
            auto mesh = m_scene.meshes()[i];
            auto& instance = instances.emplace_back();
            instance.transform = transformMatrix;
            instance.instanceCustomIndex = i;
            instance.mask = 0xFF;
            instance.instanceShaderBindingTableRecordOffset = 0;
            // @todo This is invalid in vulkan.hpp... and I don't want to use setFlags().
            // instance.flags = vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable;
            instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR; // @fixme CullDisable?
            instance.accelerationStructureReference = mesh->aft().blasHolder().deviceAddress();
        }

        // Buffer for instance data
        vulkan::BufferHolder instancesBufferHolder(m_scene.engine().impl());
        instancesBufferHolder.create(vulkan::BufferKind::AccelerationStructureInput, vulkan::BufferCpuIo::Direct, instances.size() * sizeof(vk::AccelerationStructureInstanceKHR));
        instancesBufferHolder.copy(instances.data(), instances.size() * sizeof(vk::AccelerationStructureInstanceKHR));

        vk::AccelerationStructureGeometryInstancesDataKHR geometryInstances;
        geometryInstances.arrayOfPointers = false; // @fixme IS THAT THE KEY FOR EASY INSTANCING?
        geometryInstances.data = instancesBufferHolder.deviceAddress();

        vk::AccelerationStructureGeometryKHR accelerationStructureGeometry;
        accelerationStructureGeometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
        accelerationStructureGeometry.geometryType = vk::GeometryTypeKHR::eInstances;
        accelerationStructureGeometry.geometry.instances = geometryInstances;

        topLevelAS.primitiveCount(instances.size());
        topLevelAS.create(vk::AccelerationStructureTypeKHR::eTopLevel, accelerationStructureGeometry);
    }
}

void RayTracerStage::createResources()
{

    createAS(); // The AS is binded through the descriptor below, so it can be created after or before the initPass()

    // Final
    auto finalFormat = vk::Format::eR8G8B8A8Unorm;
    m_finalImageHolder.create(vulkan::ImageKind::Storage, finalFormat, m_extent);

    descriptorHolder.updateSet(descriptorSet.get(), topLevelAS.accelerationStructure(), 0u);
    descriptorHolder.updateSet(descriptorSet.get(), vulkan::DescriptorKind::StorageImage, m_finalImageHolder.view(), m_finalImageHolder.layout(), 0u);

    // @fixme Should be done in scene?
    // indicesBuffer.create(vulkan::BufferKind::ShaderStorage, vulkan::BufferCpuIo::OnDemandStaging, m_scene.meshes().size() * sizeof(vk::Buffer));
    // verticesBuffer.create(vulkan::BufferKind::ShaderStorage, vulkan::BufferCpuIo::OnDemandStaging, m_scene.meshes().size() * sizeof(vk::Buffer));
    for (auto i = 0u; i < m_scene.meshes().size(); ++i) {
        auto mesh = m_scene.meshes()[i];
        descriptorHolder.updateSet(descriptorSet.get(), vulkan::DescriptorKind::StorageBuffer, mesh->aft().instanceBufferHolder().buffer(), mesh->aft().instanceBufferHolder().size(), 0u, i);
        descriptorHolder.updateSet(descriptorSet.get(), vulkan::DescriptorKind::StorageBuffer, mesh->aft().indexBufferHolder().buffer(), mesh->aft().indexBufferHolder().size(), 1u, i);
        descriptorHolder.updateSet(descriptorSet.get(), vulkan::DescriptorKind::StorageBuffer, mesh->aft().vertexBufferHolder().buffer(), mesh->aft().vertexBufferHolder().size(), 2u, i);
    }

    for (auto i = m_scene.meshes().size(); i < 1024; ++i) {
        auto mesh = m_scene.meshes()[0]; // @fixme Dummy buffers?
        descriptorHolder.updateSet(descriptorSet.get(), vulkan::DescriptorKind::StorageBuffer, mesh->aft().instanceBufferHolder().buffer(), mesh->aft().instanceBufferHolder().size(), 0u, i);
        descriptorHolder.updateSet(descriptorSet.get(), vulkan::DescriptorKind::StorageBuffer, mesh->aft().indexBufferHolder().buffer(), mesh->aft().indexBufferHolder().size(), 1u, i);
        descriptorHolder.updateSet(descriptorSet.get(), vulkan::DescriptorKind::StorageBuffer, mesh->aft().vertexBufferHolder().buffer(), mesh->aft().vertexBufferHolder().size(), 2u, i);
    }
}
