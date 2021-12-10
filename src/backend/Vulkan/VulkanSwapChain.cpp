
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

bool VulkanSwapChain::Acquire(void *state, VulkanRenderFrame &frame) {

    uint32_t image_index = 0;
    if (!driver->acquire(swap_chain, &image_index))
        return false;

    frame = frames[image_index];
    beginFrame(state, frame);
    return true;
}

bool VulkanSwapChain::Present(VulkanRenderFrame &frame) {
    endFrame(frame);
    return driver->present(swap_chain,1 , frame.command_buffer);
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
        FrameBufferAttachment attachments[3] = {{}, {}, {}};
        attachments[0].type = FrameBufferAttachmentType::COLOR;
        attachments[0].color.texture = color;
        //todo fix this issue
        auto vk_texture = static_cast<vulkan::Texture *>(color);
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
        frame.command_buffer = driver->createCommandBuffer(CommandBufferType::PRIMARY);
        i++;
    }

}

void VulkanSwapChain::shutdownFrames() {

    for (VulkanRenderFrame &frame: frames) {
        vkFreeDescriptorSets(context->Device(), context->DescriptorPool(), 1, &frame.descriptor_set);
        driver->unmap(frame.uniform_buffer);
        driver->destroyUniformBuffer(frame.uniform_buffer);
        driver->destroyFrameBuffer(frame.frame_buffer);
        driver->destroyCommandBuffer(frame.command_buffer);
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

void VulkanSwapChain::beginFrame(void *state, const VulkanRenderFrame &frame) {
    memcpy(frame.uniform_buffer_data, state, static_cast<size_t>(uboSize));

    vulkan::SwapChain *vk_swap_chain = static_cast<vulkan::SwapChain *>(swap_chain);
    VkFramebuffer framebuffer = static_cast<vulkan::FrameBuffer *>(frame.frame_buffer)->framebuffer;
    VkCommandBuffer command_buffer = static_cast<vulkan::CommandBuffer *>(frame.command_buffer)->command_buffer;

    driver->reset(frame.command_buffer);
    driver->begin(frame.command_buffer);


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

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanSwapChain::endFrame(const VulkanRenderFrame &frame) {
    driver->endRenderPass(frame.command_buffer);
    driver->end(frame.command_buffer);
    driver->submitSynced(frame.command_buffer,swap_chain);
}
