#include "./render-pass-holder.hpp"

#include "../render-engine-impl.hpp"
#include "./pipeline-holder.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

RenderPassHolder::RenderPassHolder(RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_renderPass(engine.device())
{
}

void RenderPassHolder::add(PipelineHolder& pipelineHolder)
{
    m_pipelineHolders.emplace_back(&pipelineHolder);
    pipelineHolder.bindRenderPass(m_renderPass);
}

void RenderPassHolder::init()
{
    m_engine.device().waitIdle();

    std::vector<vk::SubpassDescription> subpassDescriptions;
    std::vector<std::vector<vk::AttachmentReference>> attachmentsReferences(4 * m_pipelineHolders.size());
    std::vector<vk::AttachmentDescription> attachmentDescriptions;
    std::vector<vk::SubpassDependency> subpassDependencies;

    for (auto i = 0u; i < m_pipelineHolders.size(); ++i) {
        auto& pipelineHolder = *m_pipelineHolders[i];
        const auto& colorAttachments = pipelineHolder.colorAttachments();
        const auto& depthStencilAttachment = pipelineHolder.depthStencilAttachment();
        const auto& inputAttachments = pipelineHolder.inputAttachments();
        const auto& resolveAttachment = pipelineHolder.resolveAttachment();

        // Color attachments
        auto& colorAttachmentReferences = attachmentsReferences[4 * i];
        for (const auto& colorAttachment : colorAttachments) {
            vk::AttachmentReference reference;
            reference.attachment = attachmentDescriptions.size();
            reference.layout = vk::ImageLayout::eColorAttachmentOptimal;
            colorAttachmentReferences.emplace_back(reference);

            // @fixme Made colorAttachment.finalLayout useless most of the time...
            vk::AttachmentDescription description;
            description.format = colorAttachment.format;
            description.samples = vk::SampleCountFlagBits::e1;
            description.initialLayout = colorAttachment.clear ? vk::ImageLayout::eUndefined : vk::ImageLayout::eShaderReadOnlyOptimal;
            description.loadOp = colorAttachment.clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
            description.storeOp = vk::AttachmentStoreOp::eStore;
            description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            description.samples = pipelineHolder.sampleCount();
            description.finalLayout = (colorAttachment.finalLayout == vk::ImageLayout::ePresentSrcKHR)
                                          ? colorAttachment.finalLayout
                                          : vk::ImageLayout::eShaderReadOnlyOptimal;
            description.flags = vk::AttachmentDescriptionFlagBits::eMayAlias;
            attachmentDescriptions.emplace_back(description);
        }

        // Depth stencil attachment
        auto& depthStencilAttachmentReferences = attachmentsReferences[4 * i + 1];
        if (depthStencilAttachment) {
            vk::AttachmentReference reference;
            reference.attachment = attachmentDescriptions.size();
            reference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            depthStencilAttachmentReferences.emplace_back(reference);

            vk::AttachmentDescription description;
            description.format = depthStencilAttachment->format;
            description.samples = vk::SampleCountFlagBits::e1;
            description.loadOp = depthStencilAttachment->clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
            description.storeOp = vk::AttachmentStoreOp::eStore;
            description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            description.samples = pipelineHolder.sampleCount();
            description.initialLayout =
                depthStencilAttachment->clear ? vk::ImageLayout::eUndefined : vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            description.finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            attachmentDescriptions.emplace_back(description);
        }

        // Input attachments
        auto& inputAttachmentReferences = attachmentsReferences[4 * i + 2];
        for (const auto& inputAttachment : inputAttachments) {
            vk::AttachmentReference reference;
            reference.attachment = attachmentDescriptions.size();
            reference.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            inputAttachmentReferences.emplace_back(reference);

            vk::AttachmentDescription description;
            description.format = inputAttachment.format;
            description.samples = vk::SampleCountFlagBits::e1;
            description.loadOp = vk::AttachmentLoadOp::eDontCare;
            description.storeOp = vk::AttachmentStoreOp::eDontCare;
            description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            description.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            description.flags = vk::AttachmentDescriptionFlagBits::eMayAlias; // @fixme Useful?
            attachmentDescriptions.emplace_back(description);
        }

        // Resolve attachment
        auto& resolveAttachmentReferences = attachmentsReferences[4 * i + 3];
        if (resolveAttachment) {
            vk::AttachmentReference reference;
            reference.attachment = attachmentDescriptions.size();
            reference.layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            resolveAttachmentReferences.emplace_back(reference);

            vk::AttachmentDescription description;
            description.format = resolveAttachment->format;
            description.samples = vk::SampleCountFlagBits::e1;
            description.loadOp = vk::AttachmentLoadOp::eDontCare;
            description.storeOp = vk::AttachmentStoreOp::eStore;
            description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            description.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            attachmentDescriptions.emplace_back(description);
        }

        // Subpass
        vk::SubpassDescription subpassDescription;
        subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDescription.colorAttachmentCount = colorAttachments.size();
        subpassDescription.pColorAttachments = colorAttachmentReferences.data();
        if (depthStencilAttachment) {
            subpassDescription.pDepthStencilAttachment = depthStencilAttachmentReferences.data();
        }
        subpassDescription.inputAttachmentCount = inputAttachments.size();
        subpassDescription.pInputAttachments = inputAttachmentReferences.data();
        if (resolveAttachment) {
            subpassDescription.pResolveAttachments = resolveAttachmentReferences.data();
        }
        subpassDescriptions.emplace_back(subpassDescription);

        if (pipelineHolder.selfDependent()) {
            vk::SubpassDependency subpassDependency;
            subpassDependency.srcSubpass = i;
            subpassDependency.dstSubpass = i;
            subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
            subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
            subpassDependencies.emplace_back(subpassDependency);
        }

        vk::SubpassDependency subpassDependency;
        subpassDependency.srcSubpass = (i == 0u) ? VK_SUBPASS_EXTERNAL : i - 1u;
        subpassDependency.dstSubpass = (i == m_pipelineHolders.size()) ? VK_SUBPASS_EXTERNAL : i;
        // @todo Be clever about these stage mask
        subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
        subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eAllGraphics;
        subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
                                          | vk::AccessFlagBits::eDepthStencilAttachmentWrite
                                          | vk::AccessFlagBits::eInputAttachmentRead;
        subpassDependency.dependencyFlags = vk::DependencyFlagBits::eByRegion;
        subpassDependencies.emplace_back(subpassDependency);
    }

    // The render pass indeed
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = attachmentDescriptions.size();
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = subpassDescriptions.size();
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();

    if (m_engine.device().createRenderPass(&renderPassInfo, nullptr, m_renderPass.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.render-stage") << "Failed to create render pass." << std::endl;
    }

    // Init pipeline holders
    for (auto i = 0u; i < m_pipelineHolders.size(); ++i) {
        m_pipelineHolders[i]->init(i);
    }
}
