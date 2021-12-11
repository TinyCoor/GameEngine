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

ImGuiRender::ImGuiRender(const Device *ctx,
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
    init_info.Device = context->LogicDevice();
    init_info.QueueFamily = context->GraphicsQueueFamily();
    init_info.Queue = context->GraphicsQueue();
    init_info.DescriptorPool = context->DescriptorPool();
    init_info.MSAASamples = context->getMaxSampleCount();
    init_info.MinImageCount = swapChain->getNumImages();
    init_info.ImageCount = swapChain->getNumImages();

    ImGui_ImplVulkan_Init(&init_info,swapChain->getDummyRenderPass());

    VkCommandBuffer imGuiCommandBuffer = VulkanUtils::beginSingleTimeCommands(context);
    ImGui_ImplVulkan_CreateFontsTexture(imGuiCommandBuffer);
    VulkanUtils::endSingleTimeCommands(context, imGuiCommandBuffer);

}

void ImGuiRender::render(const VulkanRenderFrame& frame)
{
    VkCommandBuffer command_buffer =static_cast<vulkan::CommandBuffer*>(frame.command_buffer)->command_buffer;
    VkFramebuffer frame_buffer = static_cast<vulkan::FrameBuffer *>(frame.frame_buffer)->framebuffer;

    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = renderPass;
    info.framebuffer = frame_buffer;
    info.renderArea.offset = {0, 0};
    info.renderArea.extent = extent;

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

}

void ImGuiRender::shutdown() {
    ImGui_ImplVulkan_Shutdown();
    imGuiContext = nullptr;
}

void ImGuiRender::resize(VulkanSwapChain* swapChain) {
    extent = swapChain->getExtent();
    ImGui_ImplVulkan_SetMinImageCount(swapChain->getNumImages());
}
