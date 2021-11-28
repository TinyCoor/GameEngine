//
// Created by 12132 on 2021/11/7.
//

#include "VulkanImGuiRender.h"
#include "../../app/VulkanRenderScene.h"
#include "VulkanUtils.h"
#include "../../app/RenderState.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "Macro.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
using namespace render::backend::vulkan;

ImGuiRender::ImGuiRender(const VulkanContext *ctx,
                                     ImGuiContext* imgui_ctx,
                                     VkExtent2D size,
                                     VkRenderPass pass)
    : context(ctx),
      imGuiContext(imgui_ctx),
      extent(size),
      renderPass(pass)
{

}

ImGuiRender::~ImGuiRender()
{
    shutdown();
}

void ImGuiRender::init(VulkanSwapChain* swapChain)
{

    // Init ImGui bindings for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = context->Instance();
    init_info.PhysicalDevice = context->PhysicalDevice();
    init_info.Device = context->Device();
    init_info.QueueFamily = context->GraphicsQueueFamily();
    init_info.Queue = context->GraphicsQueue();
    init_info.DescriptorPool = context->DescriptorPool();
    init_info.MSAASamples = context->MaxMSAASamples();
    init_info.MinImageCount = swapChain->getNumImages();
    init_info.ImageCount = swapChain->getNumImages();

    ImGui_ImplVulkan_Init(&init_info,swapChain->getRenderPass());

    VkCommandBuffer imGuiCommandBuffer = VulkanUtils::beginSingleTimeCommands(context);
    ImGui_ImplVulkan_CreateFontsTexture(imGuiCommandBuffer);
    VulkanUtils::endSingleTimeCommands(context, imGuiCommandBuffer);

}

void ImGuiRender::render(const VulkanRenderFrame& frame)
{
    VkCommandBuffer commandBuffer = frame.commandBuffer;
    VkFramebuffer frameBuffer = frame.frameBuffer;

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = frameBuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    std::array<VkClearValue, 3> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[2].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);

}

void ImGuiRender::shutdown() {
    ImGui_ImplVulkan_Shutdown();
    imGuiContext = nullptr;
}

void ImGuiRender::resize(VulkanSwapChain* swapChain) {
    extent = swapChain->getExtent();
    ImGui_ImplVulkan_SetMinImageCount(swapChain->getNumImages());
}
