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


VulkanImGuiRender::VulkanImGuiRender(const VulkanRenderContext &ctx)
:context(ctx)
{

}

VulkanImGuiRender::~VulkanImGuiRender()
{
}

void VulkanImGuiRender::init(RenderState &state,
                             VulkanRenderScene *scene,
                             const VulkanSwapChain* swapChain)
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

    //ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
//    VK_CHECK(vkEndCommandBuffer(commandBuffer),"Can't record command buffer");
}

void VulkanImGuiRender::shutdown() {
    ImGui_ImplGlfw_Shutdown();
}
