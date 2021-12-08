
#include "VulkanSwapChain.h"
#include "VulkanUtils.h"
#include "../../app/VulkanApplication.h"
#include "Macro.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "VulkanRenderPassBuilder.h"
#include <functional>
#include <GLFW/glfw3.h>
#include <limits>
#include <cassert>

using namespace render::backend::vulkan;

VulkanSwapChain::VulkanSwapChain(render::backend::Driver *driver, void *nativeWindow, VkDeviceSize uboSize)
    : driver(driver),
      native_window(nativeWindow),
      uboSize(uboSize) {
  context = static_cast<render::backend::vulkan::VulkanDriver *>(driver)->GetVulkanContext();

}

VulkanSwapChain::~VulkanSwapChain() {
}

void VulkanSwapChain::init(int width, int height) {
  swapChain = driver->createSwapChain(native_window, width, height);
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swapChain);
  initPersistent(vk_swap_chain->surface_format.format);
  initTransient(width, height, vk_swap_chain->surface_format.format);
  initFrames(uboSize, vk_swap_chain->sizes.width, vk_swap_chain->sizes.height,
             vk_swap_chain->num_images, vk_swap_chain->views);

}

void VulkanSwapChain::reinit(int width, int height) {

  shutdownTransient();
  shutdownFrames();

  driver->destroySwapChain(swapChain);
  swapChain = driver->createSwapChain(native_window, width, height);
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swapChain);

  initTransient(width, height, vk_swap_chain->surface_format.format);
  initFrames(uboSize, width, height,
             vk_swap_chain->num_images, vk_swap_chain->views);

}

void VulkanSwapChain::shutdown() {
  shutdownTransient();
  shutdownFrames();
  shutdownPersistent();
}

bool VulkanSwapChain::Acquire(const RenderState &state, VulkanRenderFrame &frame) {
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swapChain);
  uint32_t current_frame = vk_swap_chain->current_image;

  vkWaitForFences(context->Device(), 1, &vk_swap_chain->rendering_finished_cpu[current_frame],
                  VK_TRUE, std::numeric_limits<uint64_t>::max());

  VkResult result = vkAcquireNextImageKHR(
      context->Device(),
      vk_swap_chain->swapchain,
      std::numeric_limits<uint64_t>::max(),
      vk_swap_chain->image_available_gpu[current_frame],
      VK_NULL_HANDLE,
      &vk_swap_chain->current_image
  );

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return false;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("Can't aquire swap chain image");

  frame = frames[vk_swap_chain->current_image];

  memcpy(frame.uniformBufferData, &state, uboSize);

  //reset command buffer
  VK_CHECK(vkResetCommandBuffer(frame.commandBuffer, 0), "Can't Reset Command Buffer");
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  VK_CHECK(vkBeginCommandBuffer(frame.commandBuffer, &beginInfo), "Can't begin recording command buffer");

  return true;
}

bool VulkanSwapChain::Present(VulkanRenderFrame &frame) {
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swapChain);
  uint32_t current_frame = vk_swap_chain->current_image;

  VK_CHECK(vkEndCommandBuffer(frame.commandBuffer), "Can't record command buffer");

  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &vk_swap_chain->image_available_gpu[current_frame];
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &frame.commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &vk_swap_chain->render_finished_gpu[current_frame];

  vkResetFences(context->Device(), 1, &vk_swap_chain->rendering_finished_cpu[current_frame]);
  VK_CHECK(vkQueueSubmit(context->GraphicsQueue(), 1, &submitInfo,
                         vk_swap_chain->rendering_finished_cpu[current_frame]),
           "Can't submit command buffer");

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &vk_swap_chain->render_finished_gpu[current_frame];
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &vk_swap_chain->swapchain;
  presentInfo.pImageIndices = &vk_swap_chain->current_image;
  presentInfo.pResults = nullptr; // Optional

  VulkanUtils::transitionImageLayout(
      context,
      vk_swap_chain->images[current_frame],
      vk_swap_chain->surface_format.format,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  );

  auto result = vkQueuePresentKHR(vk_swap_chain->present_queue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    return false;
  else if (result != VK_SUCCESS)
    throw std::runtime_error("Can't aquire swap chain image");

  vk_swap_chain->current_image++;
  vk_swap_chain->current_image %= vk_swap_chain->num_images;

  return true;
}

void VulkanSwapChain::initFrames(VkDeviceSize uboSize, uint32_t width, uint32_t height,
                                 uint32_t num_images, VkImageView *views) {

  frames.resize(num_images);
  int i = 0;
  for (auto &frame: frames) {
    //create Uniform Buffer Object
    VulkanUtils::createBuffer(
        context,
        uboSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        frame.uniformBuffers,
        frame.uniformBuffersMemory
    );

    vkMapMemory(context->Device(), frame.uniformBuffersMemory, 0, uboSize, 0, &frame.uniformBufferData);

    //Create descriptor Set
    VkDescriptorSetAllocateInfo swapchainDescriptorSetAllocInfo = {};
    swapchainDescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    swapchainDescriptorSetAllocInfo.descriptorPool = context->DescriptorPool();
    swapchainDescriptorSetAllocInfo.descriptorSetCount = 1;
    swapchainDescriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(context->Device(),
                                      &swapchainDescriptorSetAllocInfo,
                                      &frame.swapchainDescriptorSet),
             "Can't allocate swap chain descriptor sets");

    VulkanUtils::bindUniformBuffer(
        context->Device(),
        frame.swapchainDescriptorSet,
        0,
        frame.uniformBuffers,
        0,
        sizeof(RenderState)
    );
    std::array<VkImageView, 3> attachments = {
        colorImageView,
        views[i],
        depthImageView,
    };
    //create Frame buffer
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(context->Device(), &framebufferInfo, nullptr, &frame.frameBuffer),
             "Can't create framebuffer");

    // Create command buffers
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context->CommandPool();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(context->Device(), &allocateInfo, &frame.commandBuffer),
             "Can't create command buffers");

    i++;
  }

}

void VulkanSwapChain::shutdownFrames() {

  for (int i = 0; i < frames.size(); ++i) {
    vkDestroyFramebuffer(context->Device(), frames[i].frameBuffer, nullptr);
    vkDestroyBuffer(context->Device(), frames[i].uniformBuffers, nullptr);
    vkFreeMemory(context->Device(), frames[i].uniformBuffersMemory, nullptr);
    vkFreeCommandBuffers(context->Device(), context->CommandPool(), 1, &frames[i].commandBuffer);
    vkFreeDescriptorSets(context->Device(), context->DescriptorPool(), 1, &frames[i].swapchainDescriptorSet);
    frames[i].commandBuffer = VK_NULL_HANDLE;
    frames[i].uniformBuffers = VK_NULL_HANDLE;
    frames[i].frameBuffer = VK_NULL_HANDLE;
    frames[i].uniformBuffersMemory = VK_NULL_HANDLE;
    frames[i].commandBuffer = VK_NULL_HANDLE;
    frames[i].swapchainDescriptorSet = VK_NULL_HANDLE;
  }
  frames.clear();

}

void VulkanSwapChain::initPersistent(VkFormat image_format) {
  assert(native_window);
  //create descriptor set layout and render pass
  depthFormat = VulkanUtils::selectOptimalImageFormat(context->PhysicalDevice());
  VulkanDescriptorSetLayoutBuilder swapchainDescriptorSetLayoutBuilder(context);
  descriptorSetLayout = swapchainDescriptorSetLayoutBuilder
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
      .build();

  VulkanRenderPassBuilder renderPassBuilder(context);
  renderPass = renderPassBuilder.addColorAttachment(image_format, context->MaxMSAASamples(),
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
      .addColorResolveAttachment(image_format,
                                 VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE)
      .addDepthStencilAttachment(depthFormat, context->MaxMSAASamples(), VK_ATTACHMENT_LOAD_OP_CLEAR)
      .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
      .addColorAttachmentReference(0, 0)
      .addColorResolveAttachmentReference(0, 1)
      .setDepthStencilAttachment(0, 2)
      .build();

  VulkanRenderPassBuilder noClearRenderPassBuilder(context);
  noClearRenderPass = noClearRenderPassBuilder
      .addColorAttachment(image_format, context->MaxMSAASamples(),
                          VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE)
      .addColorResolveAttachment(image_format,
                                 VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE)
      .addDepthStencilAttachment(depthFormat, context->MaxMSAASamples())
      .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
      .addColorAttachmentReference(0, 0)
      .addColorResolveAttachmentReference(0, 1)
      .setDepthStencilAttachment(0, 2)
      .build();

}

void VulkanSwapChain::shutdownPersistent() {

  vkDestroyRenderPass(context->Device(), renderPass, nullptr);
  renderPass = VK_NULL_HANDLE;

  vkDestroyRenderPass(context->Device(), noClearRenderPass, nullptr);
  noClearRenderPass = VK_NULL_HANDLE;

  vkDestroyDescriptorSetLayout(context->Device(), descriptorSetLayout, nullptr);
  descriptorSetLayout = VK_NULL_HANDLE;

}

void VulkanSwapChain::initTransient(int width, int height, VkFormat image_format) {

  //Create Color Image ImageView
  VulkanUtils::createImage2D(context,
                             width,
                             height,
                             1,
                             context->MaxMSAASamples(),
                             image_format,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             colorImage, colorImageMemory
  );

  colorImageView = VulkanUtils::createImageView(context->Device(),
                                                colorImage,
                                                image_format,
                                                VK_IMAGE_ASPECT_COLOR_BIT,
                                                VK_IMAGE_VIEW_TYPE_2D);
  VulkanUtils::transitionImageLayout(context,
                                     colorImage,
                                     image_format,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  //Create Depth Buffer
  depthFormat = VulkanUtils::selectOptimalImageFormat(context->PhysicalDevice());
  VulkanUtils::createImage2D(context,
                             width,
                             height,
                             1,
                             context->MaxMSAASamples(),
                             depthFormat,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             depthImage, depthImageMemory
  );

  depthImageView = VulkanUtils::createImageView(context->Device(),
                                                depthImage,
                                                depthFormat,
                                                VK_IMAGE_ASPECT_DEPTH_BIT,
                                                VK_IMAGE_VIEW_TYPE_2D);

  VulkanUtils::transitionImageLayout(context,
                                     depthImage,
                                     depthFormat,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
void VulkanSwapChain::shutdownTransient() {
  vkDestroyImage(context->Device(), depthImage, nullptr);
  depthImage = VK_NULL_HANDLE;
  vkDestroyImageView(context->Device(), depthImageView, nullptr);
  depthImageView = VK_NULL_HANDLE;
  vkFreeMemory(context->Device(), depthImageMemory, nullptr);
  depthImageMemory = VK_NULL_HANDLE;

  vkDestroyImage(context->Device(), colorImage, nullptr);
  colorImage = VK_NULL_HANDLE;
  vkDestroyImageView(context->Device(), colorImageView, nullptr);
  colorImageView = VK_NULL_HANDLE;
  vkFreeMemory(context->Device(), colorImageMemory, nullptr);
  colorImageMemory = VK_NULL_HANDLE;

}
VkExtent2D VulkanSwapChain::getExtent() const {
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swapChain);
  return vk_swap_chain->sizes;
}
uint32_t VulkanSwapChain::getNumImages() const {
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swapChain);
  return vk_swap_chain->num_images;
}

