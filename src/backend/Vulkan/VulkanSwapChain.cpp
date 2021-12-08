
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
  swap_chain = driver->createSwapChain(native_window, width, height);
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swap_chain);
  initPersistent(vk_swap_chain->surface_format.format);
  initTransient(width, height, vk_swap_chain->surface_format.format);
  initFrames(uboSize, vk_swap_chain->sizes.width, vk_swap_chain->sizes.height,
             vk_swap_chain->num_images);

}

void VulkanSwapChain::reinit(int width, int height) {

  shutdownTransient();
  shutdownFrames();

  driver->destroySwapChain(swap_chain);
  swap_chain = driver->createSwapChain(native_window, width, height);
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swap_chain);

  initTransient(width, height, vk_swap_chain->surface_format.format);
  initFrames(uboSize, width, height,
             vk_swap_chain->num_images);

}

void VulkanSwapChain::shutdown() {
  shutdownTransient();
  shutdownFrames();
  shutdownPersistent();
}

bool VulkanSwapChain::Acquire(void* state, VulkanRenderFrame &frame) {

  uint32_t image_index = 0;
  if (!driver->acquire(swap_chain, &image_index))
    return false;

  frame = frames[image_index];
  beginFrame(state, frame);
  return true;
}

bool VulkanSwapChain::Present(VulkanRenderFrame &frame) {
  endFrame(frame);
  return driver->present(swap_chain);
}

void VulkanSwapChain::initFrames(VkDeviceSize uboSize, uint32_t width, uint32_t height,
                                 uint32_t num_images) {
  frames.resize(num_images);
  int i = 0;
  for (auto &frame: frames) {
    //create Uniform Buffer Object
    frame.uniform_buffer = driver->createUniformBuffer(BufferType::DYNAMIC, static_cast<uint32_t>(uboSize));
    frame.uniform_buffer_data = driver->map(frame.uniform_buffer);

    //Create descriptor Set
    VkDescriptorSetAllocateInfo swapchainDescriptorSetAllocInfo = {};
    swapchainDescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    swapchainDescriptorSetAllocInfo.descriptorPool = context->DescriptorPool();
    swapchainDescriptorSetAllocInfo.descriptorSetCount = 1;
    swapchainDescriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(context->Device(),
                                      &swapchainDescriptorSetAllocInfo,
                                      &frame.descriptor_set),
             "Can't allocate swap chain descriptor sets");
    VkBuffer ubo = static_cast<vulkan::UniformBuffer *>(frame.uniform_buffer)->buffer;

    VulkanUtils::bindUniformBuffer(
        context->Device(),
        frame.descriptor_set,
        0,
        ubo,
        0,
        sizeof(RenderState)
    );


    //create Frame buffer
    FrameBufferAttachment attachments[3] = { {}, {}, {} };
    attachments[0].type = FrameBufferAttachmentType::COLOR;
    attachments[0].color.texture = color;
    //todo fix this issue
    auto vk_texture = static_cast<vulkan::Texture*>(color);
    attachments[0].color.num_layers = vk_texture->num_layers;
    attachments[0].color.base_mip = 0;
    attachments[0].color.num_mips = vk_texture->num_mipmaps;
    attachments[0].color.base_layer = 0;

    attachments[1].type = FrameBufferAttachmentType::SWAP_CHAIN_COLOR;
    attachments[1].swap_chain_color.swap_chain = swap_chain;
    attachments[1].swap_chain_color.base_image = i;
    attachments[1].swap_chain_color.resolve_attachment = true;
    attachments[2].type = FrameBufferAttachmentType::DEPTH;
    attachments[2].depth.texture = depth;

    frame.frame_buffer = driver->createFrameBuffer(3, attachments);

    // Create command buffers
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = context->CommandPool();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(context->Device(), &allocateInfo, &frame.command_buffer),
             "Can't create command buffers");

    i++;
  }

}

void VulkanSwapChain::shutdownFrames() {

  for (VulkanRenderFrame &frame : frames)
  {
    vkFreeCommandBuffers(context->Device(), context->CommandPool(), 1, &frame.command_buffer);
    vkFreeDescriptorSets(context->Device(), context->DescriptorPool(), 1, &frame.descriptor_set);

    driver->unmap(frame.uniform_buffer);
    driver->destroyUniformBuffer(frame.uniform_buffer);
    driver->destroyFrameBuffer(frame.frame_buffer);
  }
  frames.clear();

}

void VulkanSwapChain::initPersistent(VkFormat image_format) {
  assert(native_window);

  depth_format = driver->getOptimalDepthFormat();
  render::backend::Multisample samples = driver->getMaxSampleCount();

  auto *vk_driver = reinterpret_cast<render::backend::vulkan::VulkanDriver *>(driver);
  VkFormat vk_depth_format = vk_driver->toFormat(depth_format);
  VkSampleCountFlagBits vk_samples = vk_driver->toMultisample(samples);

  //create descriptor set layout and render pass
  VulkanDescriptorSetLayoutBuilder swapchainDescriptorSetLayoutBuilder(context);
  descriptorSetLayout = swapchainDescriptorSetLayoutBuilder
      .addDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
      .build();

  VulkanRenderPassBuilder renderPassBuilder(context);
  render_pass = renderPassBuilder.addColorAttachment(image_format, vk_samples,
                                                    VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
      .addColorResolveAttachment(image_format,
                                 VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE)
      .addDepthStencilAttachment(vk_depth_format, vk_samples, VK_ATTACHMENT_LOAD_OP_CLEAR)
      .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
      .addColorAttachmentReference(0, 0)
      .addColorResolveAttachmentReference(0, 1)
      .setDepthStencilAttachmentReference(0, 2)
      .build();

}

void VulkanSwapChain::shutdownPersistent() {

  vkDestroyRenderPass(context->Device(), render_pass, nullptr);
  render_pass = VK_NULL_HANDLE;

  vkDestroyDescriptorSetLayout(context->Device(), descriptorSetLayout, nullptr);
  descriptorSetLayout = VK_NULL_HANDLE;

}

void VulkanSwapChain::initTransient(int width, int height, VkFormat image_format) {
  auto *vk_driver = reinterpret_cast<render::backend::vulkan::VulkanDriver *>(driver);

  render::backend::Multisample max_samples = driver->getMaxSampleCount();
  render::backend::Format format = vk_driver->fromFormat(image_format);

  color = driver->createTexture2D(width, height, 1, format, max_samples);
  depth = driver->createTexture2D(width, height, 1, depth_format, max_samples);

}
void VulkanSwapChain::shutdownTransient() {
  driver->destroyTexture(color);
  color = nullptr;

  driver->destroyTexture(depth);
  depth = nullptr;

}
VkExtent2D VulkanSwapChain::getExtent() const {
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swap_chain);
  return vk_swap_chain->sizes;
}

uint32_t VulkanSwapChain::getNumImages() const {
  auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swap_chain);
  return vk_swap_chain->num_images;
}

void VulkanSwapChain::beginFrame(void *state, const VulkanRenderFrame &frame)
{
  memcpy(frame.uniform_buffer_data, state, static_cast<size_t>(uboSize));

  vulkan::SwapChain *vk_swap_chain = static_cast<vulkan::SwapChain *>(swap_chain);
  VkFramebuffer framebuffer = static_cast<vulkan::FrameBuffer *>(frame.frame_buffer)->framebuffer;

  // reset command buffer
  if (vkResetCommandBuffer(frame.command_buffer, 0) != VK_SUCCESS)
    throw std::runtime_error("Can't reset command buffer");

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  if (vkBeginCommandBuffer(frame.command_buffer, &beginInfo) != VK_SUCCESS)
    throw std::runtime_error("Can't begin recording command buffer");

  VkRenderPassBeginInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = render_pass;
  render_pass_info.framebuffer = framebuffer;
  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = vk_swap_chain->sizes;

  std::array<VkClearValue, 3> clear_values = {};
  clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clear_values[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clear_values[2].depthStencil = {1.0f, 0};
  render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
  render_pass_info.pClearValues = clear_values.data();

  vkCmdBeginRenderPass(frame.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanSwapChain::endFrame(const VulkanRenderFrame &frame)
{
  vulkan::SwapChain *vk_swap_chain = static_cast<vulkan::SwapChain *>(swap_chain);
  uint32_t current_frame = vk_swap_chain->current_image;

  // driver->endRenderPass();
  vkCmdEndRenderPass(frame.command_buffer);

  if (vkEndCommandBuffer(frame.command_buffer) != VK_SUCCESS)
    throw std::runtime_error("Can't record command buffer");

  // driver->submit(command_buffer);
  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &vk_swap_chain->image_available_gpu[current_frame];
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &frame.command_buffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &vk_swap_chain->rendering_finished_gpu[current_frame];

  vkResetFences(context->Device(), 1, &vk_swap_chain->rendering_finished_cpu[current_frame]);
  if (vkQueueSubmit(context->GraphicsQueue(), 1, &submitInfo, vk_swap_chain->rendering_finished_cpu[current_frame]) != VK_SUCCESS)
    throw std::runtime_error("Can't submit command buffer");
}
