
#include "VulkanSwapChain.h"
#include <functional>
#include <cstring>

using namespace render::backend::vulkan;

VulkanSwapChain::VulkanSwapChain(render::backend::Driver *driver, void *nativeWindow, VkDeviceSize uboSize)
    : driver(driver),
      native_window(nativeWindow),
      ubo_size(uboSize) {
}

VulkanSwapChain::~VulkanSwapChain() {
}

void VulkanSwapChain::init(int width, int height) {
    swap_chain = driver->createSwapChain(native_window, width, height);
    auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swap_chain);
    initTransient(width, height, vk_swap_chain->surface_format.format);
    initFrames(ubo_size, vk_swap_chain->sizes.width, vk_swap_chain->sizes.height,
               vk_swap_chain->num_images);
}

void VulkanSwapChain::reinit(int width, int height) {

    shutdownTransient();
    shutdownFrames();

    driver->destroySwapChain(swap_chain);
    swap_chain = driver->createSwapChain(native_window, width, height);
    auto vk_swap_chain = reinterpret_cast<vulkan::SwapChain *>(swap_chain);

    initTransient(width, height, vk_swap_chain->surface_format.format);
    initFrames(ubo_size, width, height,
               vk_swap_chain->num_images);

}

void VulkanSwapChain::shutdown() {
    shutdownTransient();
    shutdownFrames();
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

        frame.bind_set = driver->createBindSet();

        driver->bindUniformBuffer(frame.bind_set,0,frame.uniform_buffer);

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
        driver->destroyBindSet(frame.bind_set);
        driver->unmap(frame.uniform_buffer);
        driver->destroyUniformBuffer(frame.uniform_buffer);
        driver->destroyFrameBuffer(frame.frame_buffer);
        driver->destroyCommandBuffer(frame.command_buffer);
    }
    frames.clear();

}

void VulkanSwapChain::initTransient(int width, int height, VkFormat image_format) {
    auto *vk_driver = reinterpret_cast<render::backend::vulkan::VulkanDriver *>(driver);

    render::backend::Multisample max_samples = driver->getMaxSampleCount();
    render::backend::Format format = vk_driver->fromFormat(image_format);
    Format depth_format = driver->getOptimalDepthFormat();
    ///todo fix
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
    memcpy(frame.uniform_buffer_data, state, static_cast<size_t>(ubo_size));

    driver->resetCommandBuffer(frame.command_buffer);
    driver->beginCommandBuffer(frame.command_buffer);
    std::array<RenderPassClearValue, 3> clear_values = {};
    clear_values[0].color = {0.2f, 0.2f, 0.2f, 1.0f};
    clear_values[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[2].depth_stencil = {1.0f, 0};
    std::array<RenderPassLoadOp,3> load_ops{RenderPassLoadOp::CLEAR,RenderPassLoadOp::DONT_CARE,RenderPassLoadOp::CLEAR};
    std::array<RenderPassStoreOp,3> store_ops{RenderPassStoreOp::STORE,RenderPassStoreOp::STORE,RenderPassStoreOp::DONT_CARE};
    RenderPassInfo info;
    info.load_ops = load_ops.data();
    info.store_ops = store_ops.data();
    info.clear_value = reinterpret_cast<RenderPassClearValue *>(clear_values.data());
    driver->beginRenderPass(frame.command_buffer,frame.frame_buffer,&info);

}

void VulkanSwapChain::endFrame(const VulkanRenderFrame &frame) {
    driver->endRenderPass(frame.command_buffer);
    driver->endCommandBuffer(frame.command_buffer);
    driver->submitSynced(frame.command_buffer,swap_chain);
}

VkRenderPass VulkanSwapChain::getDummyRenderPass() const
{
    return static_cast<vulkan::FrameBuffer*>(frames[0].frame_buffer)->dummy_render_pass;
}
