//
// Created by y123456 on 2021/10/21.
//

#include "VulkanRenderPassBuilder.h"
#include <stdexcept>
VulkanRenderPassBuilder &
VulkanRenderPassBuilder::addColorAttachment(VkFormat format, VkSampleCountFlagBits msaaSamples) {
    // Create render pass
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = format;
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments.push_back(colorAttachment);

    return *this;
}

VulkanRenderPassBuilder &
VulkanRenderPassBuilder::addDepthStencilAttachment(VkFormat format, VkSampleCountFlagBits msaaSamples) {
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = format;
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments.push_back(depthAttachment);
    return *this;
}

VulkanRenderPassBuilder& VulkanRenderPassBuilder::addColorResolveAttachment(VkFormat format)
{
    VkAttachmentDescription colorAttachmentResolve = {};
    colorAttachmentResolve.format = format;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments.push_back(colorAttachmentResolve);
    return *this;
}

VulkanRenderPassBuilder &VulkanRenderPassBuilder::addSubpass(VkPipelineBindPoint pipelineBindPoint) {
    VkSubpassDescription info = {};
    info.pipelineBindPoint = pipelineBindPoint;

    infos.emplace_back(info);
    subpassDatas.emplace_back(SubpassData());

    return *this;
}

VulkanRenderPassBuilder &VulkanRenderPassBuilder::addColorAttachmentReference(int subpassIndex,
                                                                              int attachmentIndex) {

    if(subpassIndex < 0 || subpassIndex > subpassDatas.size()){
        return *this;
    }
    if(attachmentIndex < 0 || attachmentIndex >= attachments.size()){
        return *this;
    }

    SubpassData& subpass = subpassDatas[subpassIndex];

    VkAttachmentReference reference = {};
    reference.attachment = attachmentIndex;
    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentReferences.emplace_back(reference);

    return *this;
}

VulkanRenderPassBuilder &
VulkanRenderPassBuilder::addColorResolveAttachmentReference(int subpassIndex,
                                                            int attachmentIndex) {

    if(subpassIndex < 0 || subpassIndex > subpassDatas.size()){
        return *this;
    }
    if(attachmentIndex < 0 || attachmentIndex >= attachments.size()){
        return *this;
    }

    VkAttachmentReference reference = {};
    reference.attachment = attachmentIndex;
    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    SubpassData& subpass = subpassDatas[subpassIndex];
    subpass.colorAttachmentResolveReferences.emplace_back(reference);
    return *this;
}

VulkanRenderPassBuilder &VulkanRenderPassBuilder::setDepthStencilAttachment(int subpassIndex,
                                                                            int attachmentIndex) {

    if(subpassIndex < 0 || subpassIndex > subpassDatas.size()){
        return *this;
    }
    if(attachmentIndex < 0 || attachmentIndex >= attachments.size()){
        return *this;
    }
    SubpassData& data = subpassDatas[subpassIndex];
    if(data.depthStencilAttachmentReference == nullptr){
        data.depthStencilAttachmentReference = new VkAttachmentReference();
    }

    *(data.depthStencilAttachmentReference) = {};
    data.depthStencilAttachmentReference->attachment = attachmentIndex;
    data.depthStencilAttachmentReference->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return *this;
}


VkRenderPass VulkanRenderPassBuilder::build() {
    for (int i = 0; i < subpassDatas.size(); ++i) {
        SubpassData &data = subpassDatas[i];
        infos[i].pDepthStencilAttachment = data.depthStencilAttachmentReference;
        infos[i].colorAttachmentCount = data.colorAttachmentReferences.size();
        infos[i].pColorAttachments = data.colorAttachmentReferences.data();
        infos[i].pResolveAttachments = data.colorAttachmentResolveReferences.data();
    }

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = infos.size();
    renderPassInfo.pSubpasses = infos.data();

    if (vkCreateRenderPass(context.device_, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("Can't create render pass");

    return renderPass;

}