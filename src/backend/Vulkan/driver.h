//
// Created by 12132 on 2021/11/27.
//

#ifndef GAMEENGINE_DRIVER_H
#define GAMEENGINE_DRIVER_H
#include "../API.h"
#include <volk.h>
namespace render::backend::vulkan {

struct VertexBuffer : public render::backend::VertexBuffer {
    static constexpr int MAX_ATTRIBUTES = 10;
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    uint16_t vertex_size{0};
    uint32_t num_vertices{0};
    uint8_t num_attributes{0};
    VkVertexInputAttributeDescription attributes[MAX_ATTRIBUTES];

};

struct IndexBuffer : public render::backend::IndexBuffer {
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkIndexType type{VK_INDEX_TYPE_UINT16};
    uint32_t num_indices;
};

struct RenderPrimitive : public render::backend::RenderPrimitive {
    VkPrimitiveTopology type{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    const VertexBuffer *vertexBuffer{nullptr};
    const IndexBuffer *indexBuffer{nullptr};
};

struct BindSet: public render::backend::BindSet
{
    enum {
        MAX_BINDINGS = 32,
    };

    //todo std::variant

    union BindData{
        struct Texture{
            VkImageView view;
            VkSampler sampler;
        } texture;
        VkBuffer ubo;
    };

    VkDescriptorSetLayoutBinding bindings[MAX_BINDINGS];
    BindData bind_data[MAX_BINDINGS];
    bool bind_used[MAX_BINDINGS];

};

struct Texture : public render::backend::Texture {
    VkImage image{VK_NULL_HANDLE};
    VkImageView view{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
    VkDeviceMemory imageMemory{VK_NULL_HANDLE};
    VkImageType type{VK_IMAGE_TYPE_2D};
    VkFormat format{VK_FORMAT_R8G8B8A8_UNORM};
    size_t width{0};
    size_t height{0};
    size_t depth{0};
    uint8_t num_layers{0};
    uint8_t num_mipmaps{0};
    VkImageTiling tiling{VK_IMAGE_TILING_OPTIMAL};
    VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};
    VkImageCreateFlags flags{0};
};

struct CommandBuffer : public render::backend::CommandBuffer {
    VkCommandBuffer command_buffer{VK_NULL_HANDLE};
    uint8_t level{0};
    VkSemaphore rendering_finished_gpu{VK_NULL_HANDLE};
    VkFence rendering_finished_cpu{VK_NULL_HANDLE};
};


struct FrameBuffer : public render::backend::FrameBuffer {
    enum {
        MAX_ATTACHMENTS = 16,
    };

    VkFramebuffer framebuffer{VK_NULL_HANDLE};
    VkExtent2D sizes{0,0};

    VkRenderPass dummy_render_pass{VK_NULL_HANDLE}; // TODO: move to render pass cache

    uint8_t num_attachments{0};
    VkImageView attachments[FrameBuffer::MAX_ATTACHMENTS];
    FrameBufferAttachmentType attachment_types[FrameBuffer::MAX_ATTACHMENTS];
    VkFormat attachment_format[FrameBuffer::MAX_ATTACHMENTS];
    bool attachment_resolve[FrameBuffer::MAX_ATTACHMENTS];
    VkSampleCountFlagBits attachment_samples[FrameBuffer::MAX_ATTACHMENTS];


    // TODO: add info about attachment type (color, color resolve, depth)
};

struct UniformBuffer : public render::backend::UniformBuffer {
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    uint32_t size{0};
    void *pointer{nullptr};
};

struct Shader : public render::backend::Shader {
    ShaderType type{ShaderType::FRAGMENT};
    VkShaderModule shaderModule{VK_NULL_HANDLE};

};

struct GraphicsProgram : public render::backend::GraphicsProgram {
    const Shader *vertex;
    const Shader *tessellation_control;
    const Shader *tessellation_evaulation;
    const Shader *geometry;
    const Shader *fragment;
};

struct SwapChain : public render::backend::SwapChain {
    enum {
        MAX_IMAGES = 8,
    };

    VkSwapchainKHR swap_chain{nullptr};
    VkExtent2D sizes{0, 0};

    VkSurfaceKHR surface{nullptr};
    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkSurfaceFormatKHR surface_format;

    uint32_t present_queue_family{0xFFFF};
    VkQueue present_queue{VK_NULL_HANDLE};
    VkPresentModeKHR present_mode{VK_PRESENT_MODE_FIFO_KHR};

    VkImage msaa_color_image{VK_NULL_HANDLE};
    VkImageView msaa_color_view{VK_NULL_HANDLE};
    VkDeviceMemory msaa_color_memory{VK_NULL_HANDLE};

    VkImage depth_image{VK_NULL_HANDLE};
    VkImageView depth_view{VK_NULL_HANDLE};
    VkDeviceMemory depth_memory{VK_NULL_HANDLE};

    VkRenderPass dummy_render_pass{VK_NULL_HANDLE}; // TODO: move to render pass cache

    uint32_t num_images{0};
    uint32_t current_image{0};

    VkSemaphore image_available_gpu[SwapChain::MAX_IMAGES];
    VkFramebuffer framebuffer[SwapChain::MAX_IMAGES];
    VkImage images[SwapChain::MAX_IMAGES];
    VkImageView views[SwapChain::MAX_IMAGES];
};

class Device;
class VulkanRenderPassCache;
class DescriptorSetLayoutCache;
class DescriptorSetCache;
class VulkanDriver : public render::backend::Driver {
    Device *context{nullptr};
    VulkanRenderPassCache* render_pass_cache{nullptr};
    DescriptorSetLayoutCache* descriptor_set_layout_cache{nullptr};
    DescriptorSetCache* descriptor_set_cache{nullptr};
public:
    VulkanDriver(const char *app_name, const char *engine_name);
    ~VulkanDriver();

    Device *GetDevice() const { return context; }

    VertexBuffer *createVertexBuffer(
        BufferType type,
        uint16_t vertex_size,
        uint32_t num_vertices,
        uint8_t num_attributes,
        const VertexAttribute *attributes,
        const void *data
    ) override;

    IndexBuffer *createIndexBuffer(
        BufferType type,
        IndexSize index_size,
        uint32_t num_indices,
        const void *data
    ) override;

    RenderPrimitive *createRenderPrimitive(
        RenderPrimitiveType type,
        const render::backend::VertexBuffer *vertex_buffer,
        const render::backend::IndexBuffer *index_buffer
    ) override;

    Texture *createTexture2D(
        uint32_t width,
        uint32_t height,
        uint32_t num_mipmaps,
        Format format,
        Multisample samples = Multisample::COUNT_1,
        const void *data = nullptr,
        uint32_t num_data_mipmaps = 1
    ) override;

    Texture *createTexture2DArray(
        uint32_t width,
        uint32_t height,
        uint32_t num_mipmaps,
        uint32_t num_layers,
        Format format,
        const void *data = nullptr,
        uint32_t num_data_mipmaps = 1,
        uint32_t num_data_layers = 1
    ) override;

    Texture *createTexture3D(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t num_mipmaps,
        Format format,
        const void *data = nullptr,
        uint32_t num_data_mipmaps = 1
    ) override;

    Texture *createTextureCube(
        uint32_t width,
        uint32_t height,
        uint32_t num_mipmaps,
        Format format,
        const void *data = nullptr,
        uint32_t num_data_mipmaps = 1
    ) override;

    FrameBuffer *createFrameBuffer(
        uint8_t num_attachments,
        const FrameBufferAttachment *attachments
    ) override;

    virtual CommandBuffer *createCommandBuffer(
        CommandBufferType type
    ) override;

    UniformBuffer *createUniformBuffer(
        BufferType type,
        uint32_t size,
        const void *data = nullptr
    ) override;

    Shader *createShaderFromSource(
        ShaderType type,
        uint32_t length,
        const char *data,
        const char *path
    ) override;

    Shader *createShaderFromBytecode(
        ShaderType type,
        uint32_t size,
        const void *data
    ) override;

    ///pipeline state
    virtual void clearShader() override;
    virtual void clearBindSet() override;
    virtual void setShader(ShaderType type,const char* shader) override;
    virtual void setBindSet(const render::backend::BindSet* set, uint32_t binding) override;
    virtual BindSet* createBindSet() override;

    void destroyVertexBuffer(render::backend::VertexBuffer *vertex_buffer) override;
    void destroyIndexBuffer(render::backend::IndexBuffer *index_buffer) override;
    void destroyRenderPrimitive(render::backend::RenderPrimitive *render_primitive) override;
    void destroyTexture(render::backend::Texture *texture) override;
    void destroyFrameBuffer(render::backend::FrameBuffer *frame_buffer) override;
    void destroyCommandBuffer(render::backend::CommandBuffer *command_buffer) override;
    void destroyUniformBuffer(render::backend::UniformBuffer *uniform_buffer) override;
    void destroyShader(render::backend::Shader *shader) override;
    void destroyBindSet(render::backend::BindSet* set) override;

public:
    Multisample getMaxSampleCount() override;
    Format getOptimalDepthFormat() override;

    // TODO: remove later
    VkSampleCountFlagBits toMultisample(Multisample samples);
    Multisample fromMultisample(VkSampleCountFlagBits samples);

    VkFormat toFormat(Format format);
    Format fromFormat(VkFormat format);

public:
    void generateTexture2DMipmaps(render::backend::Texture *texture) override;
    void *map(render::backend::UniformBuffer *uniform_buffer) override;
    void unmap(render::backend::UniformBuffer *uniform_buffer) override;
    void wait() override;
    bool acquire(render::backend::SwapChain *swapchain, uint32_t *image_index) override;
    bool present(render::backend::SwapChain *swapchain,
                 uint32_t num_wait_command_buffers,
                 const render::backend::CommandBuffer* wait_command_buffers) override;
    bool submit(render::backend::CommandBuffer* command_buffer) override;
    virtual bool submitSynced(render::backend:: CommandBuffer* command_buffer,
                              render::backend::SwapChain* wait_swap_chain) override;
    virtual bool submitSynced(render::backend::CommandBuffer* command_buffer,
                              uint32_t num_wait_command_buffers,
                              const render::backend::CommandBuffer* wait_command_buffers) override;
public:
    bool reset(
        render::backend::CommandBuffer* command_buffer
    ) override;

    bool begin(
        render::backend::CommandBuffer* command_buffer) override;

     bool end(
         render::backend::CommandBuffer* command_buffer
    ) override;

    // render pass
    void beginRenderPass(
        render::backend::CommandBuffer* command_buffer,
        const render::backend::FrameBuffer *frame_buffer,
        const RenderPassInfo* info) override;

    void endRenderPass(
       render::backend::CommandBuffer* command_buffer
    ) override;


    // bind
    void bindUniformBuffer(
       render::backend::BindSet* set,
        uint32_t unit,
        const render::backend::UniformBuffer *uniform_buffer
    ) override;

    void bindTexture(
        render::backend::BindSet* set,
        uint32_t unit,
        const render::backend::Texture *texture,
        int base_mip,
        int num_mip,
        int base_layer,
        int num_layer
    ) override;

    void bindShader(
        const render::backend::Shader *shader
    ) override;

    // draw
    void drawIndexedPrimitive(
        render::backend::CommandBuffer* command_buffer,
        const render::backend::RenderPrimitive *render_primitive
    ) override;

    void drawIndexedPrimitiveInstanced(
        render::backend::CommandBuffer* command_buffer,
        const render::backend::RenderPrimitive *primitive,
        const render::backend::VertexBuffer *instance_buffer,
        uint32_t num_instances,
        uint32_t offset
    ) override;

    SwapChain *createSwapChain(void *native_window, uint32_t width, uint32_t height) override;
    void destroySwapChain(render::backend::SwapChain *swapchain) override;


};
}

#endif //GAMEENGINE_API_H
