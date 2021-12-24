//
// Created by y123456 on 2021/10/21.
//

#include "VulkanRenderPassBuilder.h"
#include <stdexcept>
namespace render::backend::vulkan {
VulkanRenderPassBuilder &
VulkanRenderPassBuilder::addColorAttachment(VkFormat format,
                                            VkSampleCountFlagBits msaaSamples,
                                            VkImageLayout finalLayout,
                                            VkAttachmentLoadOp loadOp,
                                            VkAttachmentStoreOp storeOp,
                                            VkAttachmentLoadOp stenciLoadOp,
                                            VkAttachmentStoreOp stencilStoreOp) {
  // Create render pass
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = format;
  colorAttachment.samples = msaaSamples;
  colorAttachment.loadOp = loadOp;
  colorAttachment.storeOp = storeOp;
  colorAttachment.stencilLoadOp = stenciLoadOp;
  colorAttachment.stencilStoreOp = stencilStoreOp;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = finalLayout;

  attachments.push_back(colorAttachment);

  return *this;
}

VulkanRenderPassBuilder &
VulkanRenderPassBuilder::addDepthStencilAttachment(VkFormat format,
                                                   VkSampleCountFlagBits msaaSamples,
                                                   VkImageLayout finalLayout,
                                                   VkAttachmentLoadOp loadOp,
                                                   VkAttachmentStoreOp storeOp,
                                                   VkAttachmentLoadOp stencilOp,
                                                   VkAttachmentStoreOp stencilStoreOp) {
  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format = format;
  depthAttachment.samples = msaaSamples;
  depthAttachment.loadOp = loadOp;
  depthAttachment.storeOp = storeOp;
  depthAttachment.stencilLoadOp = stencilOp;
  depthAttachment.stencilStoreOp = stencilStoreOp;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = finalLayout;

  attachments.push_back(depthAttachment);
  return *this;
}

VulkanRenderPassBuilder &VulkanRenderPassBuilder::addColorResolveAttachment(VkFormat format,
                                                                            VkImageLayout finalLayout,
                                                                            VkAttachmentLoadOp loadOp,
                                                                            VkAttachmentStoreOp storeOp,
                                                                            VkAttachmentLoadOp stencilOp,
                                                                            VkAttachmentStoreOp stencilStoreOp) {
  VkAttachmentDescription colorAttachmentResolve = {};
  colorAttachmentResolve.format = format;
  colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachmentResolve.loadOp = loadOp;
  colorAttachmentResolve.storeOp = storeOp;
  colorAttachmentResolve.stencilLoadOp = stencilOp;
  colorAttachmentResolve.stencilStoreOp = stencilStoreOp;
  colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachmentResolve.finalLayout = finalLayout;

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

  if (subpassIndex < 0 || subpassIndex > subpassDatas.size()) {
    return *this;
  }
  if (attachmentIndex < 0 || attachmentIndex >= attachments.size()) {
    return *this;
  }

  SubpassData &subpass = subpassDatas[subpassIndex];

  VkAttachmentReference reference = {};
  reference.attachment = attachmentIndex;
  reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  subpass.colorAttachmentReferences.emplace_back(reference);

  return *this;
}

VulkanRenderPassBuilder &
VulkanRenderPassBuilder::addColorResolveAttachmentReference(int subpassIndex,
                                                            int attachmentIndex) {

  if (subpassIndex < 0 || subpassIndex > subpassDatas.size()) {
    return *this;
  }
  if (attachmentIndex < 0 || attachmentIndex >= attachments.size()) {
    return *this;
  }

  VkAttachmentReference reference = {};
  reference.attachment = attachmentIndex;
  reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  SubpassData &subpass = subpassDatas[subpassIndex];
  subpass.colorAttachmentResolveReferences.emplace_back(reference);
  return *this;
}

VulkanRenderPassBuilder &VulkanRenderPassBuilder::setDepthStencilAttachmentReference(int subpassIndex,
                                                                            int attachmentIndex) {

  if (subpassIndex < 0 || subpassIndex > subpassDatas.size()) {
    return *this;
  }
  if (attachmentIndex < 0 || attachmentIndex >= attachments.size()) {
    return *this;
  }
  SubpassData &data = subpassDatas[subpassIndex];
  if (data.depthStencilAttachmentReference == nullptr) {
    data.depthStencilAttachmentReference = new VkAttachmentReference();
  }

  *(data.depthStencilAttachmentReference) = {};
  data.depthStencilAttachmentReference->attachment = attachmentIndex;
  data.depthStencilAttachmentReference->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  return *this;
}

VkRenderPass VulkanRenderPassBuilder::build(VkDevice device) {
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

  VkRenderPass render_pass{VK_NULL_HANDLE};
  if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS)
    throw std::runtime_error("Can't create render pass");

  return render_pass;

}
}