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

ImGuiRender::ImGuiRender(render::backend::Driver *driver,
                         ImGuiContext* imgui_ctx,
                         VkExtent2D size,
                         VkRenderPass pass)
    : driver(driver),
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

    auto context = static_cast<VulkanDriver*>(driver)->GetDevice();
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
