//
// Created by 12132 on 2021/11/7.
//

#include "VulkanImGuiRender.h"
#include "VulkanRenderScene.h"
#include "VulkanUtils.h"
#include "RenderState.h"
#include "VulkanSwapChain.h"
#include "Macro.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>


VulkanImGuiRender::VulkanImGuiRender(const VulkanRenderContext &ctx,
                                     VkExtent2D size,
                                     VkRenderPass pass)
:context(ctx),extent(size),renderPass(pass)
{

}

VulkanImGuiRender::~VulkanImGuiRender()
{
}

void VulkanImGuiRender::init(RenderState &state,
                             VulkanRenderScene *scene,
                             std::shared_ptr<VulkanSwapChain> swapChain)
{
    // Init ImGui bindings for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = context.instance;
    init_info.PhysicalDevice = context.physicalDevice;
    init_info.Device = context.device_;
    init_info.QueueFamily = context.graphicsQueueFamily;
    init_info.Queue = context.graphicsQueue;
    init_info.DescriptorPool = context.descriptorPool;
    init_info.MSAASamples = context.maxMSAASamples;
    init_info.MinImageCount = swapChain->getNumImages();
    init_info.ImageCount = swapChain->getNumImages();
    init_info.Allocator = nullptr;

    //TODO Fix Bug In this Function VkCreateSampler Cause Segmentation
    ImGui_ImplVulkan_Init(&init_info,swapChain->getRenderPass());

    VulkanRenderContext imGuiContext = {};
    imGuiContext.commandPool = context.commandPool;
    imGuiContext.descriptorPool = context.descriptorPool;
    imGuiContext.device_ = context.device_;
    imGuiContext.graphicsQueue = context.graphicsQueue;
    imGuiContext.maxMSAASamples = context.maxMSAASamples;
    imGuiContext.physicalDevice = context.physicalDevice;
    imGuiContext.presentQueue = context.presentQueue;

    VkCommandBuffer imGuiCommandBuffer = VulkanUtils::beginSingleTimeCommands(imGuiContext);
    ImGui_ImplVulkan_CreateFontsTexture(imGuiCommandBuffer);
    VulkanUtils::endSingleTimeCommands(imGuiContext, imGuiCommandBuffer);

}

void VulkanImGuiRender::update(RenderState &state, VulkanRenderScene *scene)
{
    //ImGui
    static float f = 0.0f;
    static int counter = 0;
    static bool show_demo_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    ImGui::Begin("Material Parameters");

    int oldCurrentEnvironment =state.currentEnvironment;
    if(ImGui::BeginCombo("Chose your Destiny",scene->getHDRTexturePath(state.currentEnvironment))){
        for (int i = 0; i < scene->getNumHDRTextures(); ++i) {
            bool selected = (i==state.currentEnvironment);
            if (ImGui::Selectable(scene->getHDRTexturePath(i),&selected))
                state.currentEnvironment = i;
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Checkbox("Demo Window", &show_demo_window);

    ImGui::SliderFloat("Lerp User Material", &state.lerpUserValues, 0.0f, 1.0f);
    ImGui::SliderFloat("Metalness", &state.userMetalness, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &state.userRoughness, 0.0f, 1.0f);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

}

void VulkanImGuiRender::render(RenderState &state,
                                          VulkanRenderScene *scene,
                                          const VulkanRenderFrame& frame)
{
    VkCommandBuffer commandBuffer = frame.commandBuffer;
    VkFramebuffer frameBuffer = frame.frameBuffer;
    VkDeviceMemory uniformBufferMemory = frame.uniformBuffersMemory;
    VkDescriptorSet descriptorSet = frame.swapchainDescriptorSet;

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

//    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//    //ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

//    vkCmdEndRenderPass(commandBuffer);

}

void VulkanImGuiRender::shutdown() {
    ImGui_ImplGlfw_Shutdown();
}

void VulkanImGuiRender::resize(std::shared_ptr<VulkanSwapChain> swapChain) {
    extent = swapChain->getExtent();
    ImGui_ImplVulkan_SetMinImageCount(swapChain->getNumImages());
}
