//
// Created by y123456 on 2021/10/21.
//

#ifndef GAMEENGINE_VULKANRENDERPASSBUILDER_H
#define GAMEENGINE_VULKANRENDERPASSBUILDER_H
#include "VulkanRenderContext.h"

class VulkanRenderPassBuilder {
    struct SubpassData{
        std::vector<VkAttachmentReference> colorAttachmentReferences;
        std::vector<VkAttachmentReference> colorAttachmentResolveReferences;
        VkAttachmentReference* depthStencilAttachmentReference{nullptr};
    };
    std::vector<SubpassData> subpassDatas;
    std::vector<VkSubpassDescription> infos{};
    VulkanRenderContext context;
    VkRenderPass renderPass{VK_NULL_HANDLE};
    std::vector<VkAttachmentDescription> attachments;

public:
    VulkanRenderPassBuilder(const VulkanRenderContext& ctx): context(ctx){}

    inline VkRenderPass getRenderPass(){return renderPass;}

    VulkanRenderPassBuilder& addColorResolveAttachment(VkFormat format,
                                                       VkAttachmentLoadOp loadOp =VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                       VkAttachmentStoreOp storeOp =VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                       VkAttachmentLoadOp stencilOp =VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                       VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);

    VulkanRenderPassBuilder& addDepthStencilAttachment(VkFormat format,
                                                       VkSampleCountFlagBits msaaSamples,
                                                       VkAttachmentLoadOp loadOp =VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                       VkAttachmentStoreOp storeOp =VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                       VkAttachmentLoadOp stencilOp =VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                       VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
                                                       );

    VulkanRenderPassBuilder& addColorAttachment(VkFormat format,
                                                VkSampleCountFlagBits msaaSamples,
                                                VkAttachmentLoadOp loadOp =VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VkAttachmentStoreOp storeOp =VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                VkAttachmentLoadOp stencilOp =VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);

    VulkanRenderPassBuilder &addColorAttachmentReference(
            int subpassIndex,
            int attachmentIndex
            );

    VulkanRenderPassBuilder &addColorResolveAttachmentReference(
            int subpassIndex,
            int attachmentIndex
    );

    VulkanRenderPassBuilder &setDepthStencilAttachment(
            int subpassIndex,
            int attachmentIndex
    );

    VulkanRenderPassBuilder& addSubpass(VkPipelineBindPoint bindPoint);

    VkRenderPass build();

};



#endif //GAMEENGINE_VULKANRENDERPASSBUILDER_H
