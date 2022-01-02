//
// Created by 12132 on 2021/11/27.
//

#include "Device.h"
#include "Macro.h"
#include "PipelineCache.h"
#include "VulkanRenderPassCache.h"
#include "DescriptorSetLayoutCache.h"
#include "PipelineLayoutCache.h"
#include "VulkanRenderPassBuilder.h"
#include "Utils.h"
#include "auxiliary.h"
#include "context.h"
#include "driver.h"
#include "platform.h"
#include "shaderc.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace render::backend::shaderc {
static shaderc_shader_kind vulkan_to_shaderc_kind(ShaderType type)
{
    switch (type) {
        // Graphics pipeline
    case ShaderType::VERTEX: return shaderc_vertex_shader;
    case ShaderType::TESSELLATION_CONTROL: return shaderc_tess_control_shader;
    case ShaderType::TESSELLATION_EVALUATION: return shaderc_tess_evaluation_shader;
    case ShaderType::GEOMETRY: return shaderc_geometry_shader;
    case ShaderType::FRAGMENT: return shaderc_fragment_shader;

        // Compute pipeline
    case ShaderType::COMPUTE: return shaderc_compute_shader;

#if NV_EXTENSIONS
        // Raytrace pipeline
                    case ShaderType::RAY_GENERATION: return shaderc_raygen_shader;
                    case ShaderType::INTERSECTION: return shaderc_intersection_shader;
                    case ShaderType::ANY_HIT: return shaderc_anyhit_shader;
                    case ShaderType::CLOSEST_HIT: return shaderc_closesthit_shader;
                    case ShaderType::MISS: return shaderc_miss_shader;
                    case ShaderType::CALLABLE: return shaderc_callable_shader;
#endif
    }

    return shaderc_glsl_infer_from_source;
}

static shaderc_include_result *includeResolver(
    void *user_data,
    const char *requested_source,
    int type,
    const char *requesting_source,
    size_t include_depth
)
{
    shaderc_include_result *result = new shaderc_include_result();
    result->user_data = user_data;
    result->source_name = nullptr;
    result->source_name_length = 0;
    result->content = nullptr;
    result->content_length = 0;

    std::string target_dir = "";

    switch (type) {
    case shaderc_include_type_standard: {
        // TODO: remove this, not generic
        target_dir = "shaders/";
    }
        break;

    case shaderc_include_type_relative: {
        std::string_view source_path = requesting_source;
        size_t pos = source_path.find_last_of("/\\");

        if (pos != std::string_view::npos)
            target_dir = source_path.substr(0, pos + 1);
    }
        break;
    }

    std::string target_path = target_dir + std::string(requested_source);

    std::ifstream file(target_path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "shaderc::include_resolver(): can't load include at \"" << target_path << "\"" << std::endl;
        return result;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    char *buffer = new char[fileSize];

    file.seekg(0);
    file.read(buffer, fileSize);
    file.close();

    char *path = new char[target_path.size() + 1];
    memcpy(path, target_path.c_str(), target_path.size());
    path[target_path.size()] = '\x0';

    result->source_name = path;
    result->source_name_length = target_path.size() + 1;
    result->content = buffer;
    result->content_length = fileSize;

    return result;
}

static void includeResultReleaser(void *userData, shaderc_include_result *result)
{
    delete result->source_name;
    delete result->content;
    delete result;
}
}

namespace render::backend::vulkan {
VulkanDriver::VulkanDriver(const char *app_name, const char *engine_name) : device(new Device)
{
    device->init(app_name, engine_name);
    vk_context = new context;
    render_pass_cache = new VulkanRenderPassCache(device);
    descriptor_set_layout_cache = new DescriptorSetLayoutCache(device);
    pipeline_layout_cache = new PipelineLayoutCache(device, descriptor_set_layout_cache);
    pipeline_cache = new PipelineCache(device, pipeline_layout_cache);
}

VulkanDriver::~VulkanDriver() noexcept
{
    if (device != nullptr) {
        device->shutdown();
        delete device;
    }
    delete descriptor_set_layout_cache;
    descriptor_set_layout_cache = nullptr;
    delete pipeline_layout_cache;
    pipeline_layout_cache = nullptr;
    delete pipeline_cache;
    pipeline_cache = nullptr;
}

VertexBuffer *VulkanDriver::createVertexBuffer(BufferType type,
                                               uint16_t vertex_size,
                                               uint32_t num_vertices,
                                               uint8_t num_attributes,
                                               const VertexAttribute *attributes,
                                               const void *data)
{
    assert(type == BufferType::STATIC && "Dynamic are not impl");
    assert(num_vertices != 0 && data != nullptr && "Invalid data");
    assert(vertex_size != 0 && data != nullptr && "Invalid VertexSize");
    assert(num_attributes <= vulkan::VertexBuffer::MAX_ATTRIBUTES && "Vetex Attribute limit to 16");

    VkDeviceSize buffer_size = vertex_size * num_vertices;
    auto *result = new vulkan::VertexBuffer;
    result->vertex_size = vertex_size;
    result->num_vertices = num_vertices;
    result->type = type;
    result->num_attributes = num_attributes;

    for (uint32_t i = 0; i < num_attributes; ++i) {
        result->attribute_formats[i] = toFormat(attributes[i].format);
        result->attribute_offsets[i] = attributes[i].offset;
    }

    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VkMemoryPropertyFlags memory_flags = 0;

    if (type == BufferType::STATIC)
    {
        usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    else if (type == BufferType::DYNAMIC)
    {
        memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    Utils::createBuffer(device,buffer_size,usage_flags,memory_flags,result->buffer,result->memory);

    ///
    if (data)
    {
        if (type == BufferType::STATIC)
            Utils::fillDeviceLocalBuffer(device, result->buffer, buffer_size, data);
        else if (type == BufferType::DYNAMIC)
            Utils::fillHostVisibleBuffer(device, result->memory, buffer_size, data);
    }
    return result;
}

IndexBuffer *VulkanDriver::createIndexBuffer(BufferType type,
                                             IndexFormat index_size,
                                             uint32_t num_indices,
                                             const void *data)
{
    assert(num_indices != 0 && data != nullptr && "Invalid data");
    assert(static_cast<uint32_t>(index_size) != 0 && data != nullptr && "Invalid VertexSize");
    IndexBuffer *result = new IndexBuffer();
    result->num_indices = num_indices;
    result->type = type;
    result->index_type = Utils::getIndexType(index_size);

    VkDeviceSize buffer_size = Utils::getIndexSize(index_size) * num_indices;

    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkMemoryPropertyFlags memory_flags = 0;

    if (type == BufferType::STATIC)
    {
        usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    else if (type == BufferType::DYNAMIC)
    {
        memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    // create index buffer
    Utils::createBuffer(device, buffer_size, usage_flags, memory_flags, result->buffer, result->memory);

    if (data)
    {
        if (type == BufferType::STATIC)
            Utils::fillDeviceLocalBuffer(device, result->buffer, buffer_size, data);
        else if (type == BufferType::DYNAMIC)
            Utils::fillHostVisibleBuffer(device, result->memory, buffer_size, data);
    }

    return result;
}


Texture *VulkanDriver::createTexture2D(uint32_t width,
                                       uint32_t height,
                                       uint32_t num_mipmaps,
                                       Format format,
                                       Multisample samples,
                                       const void *data,
                                       uint32_t num_data_mipmaps)
{
    vulkan::Texture *texture = new vulkan::Texture();
    texture->width = width;
    texture->height = height;
    texture->depth = 1;
    texture->format = toFormat(format);
    texture->num_mipmaps = num_mipmaps;
    texture->num_layers = 1;
    texture->type = VK_IMAGE_TYPE_2D;
    texture->samples = Utils::getSamples(samples);
    texture->tiling = VK_IMAGE_TILING_OPTIMAL;
    texture->flags = 0;

    createTextureData(device, texture, format, data, num_data_mipmaps, 1);
    return texture;
}

Texture *VulkanDriver::createTexture2DArray(uint32_t width,
                                            uint32_t height,
                                            uint32_t num_mipmaps,
                                            uint32_t num_layers,
                                            Format format,
                                            const void *data,
                                            uint32_t num_data_mipmaps,
                                            uint32_t num_data_layers)
{

    vulkan::Texture *texture = new vulkan::Texture();
    texture->type = VK_IMAGE_TYPE_2D;
    texture->width = width;
    texture->height = height;
    texture->depth = 1;
    texture->format = toFormat(format);
    texture->num_mipmaps = num_mipmaps;
    texture->num_layers = num_layers;
    texture->samples = VK_SAMPLE_COUNT_1_BIT;
    texture->tiling = VK_IMAGE_TILING_OPTIMAL;
    texture->flags = 0;

    createTextureData(device, texture, format, data, num_data_mipmaps, num_data_layers);

    return texture;
}

Texture *VulkanDriver::createTexture3D(uint32_t width,
                                       uint32_t height,
                                       uint32_t depth,
                                       uint32_t num_mipmaps,
                                       Format format,
                                       const void *data,
                                       uint32_t num_data_mipmaps)
{
    vulkan::Texture *texture = new vulkan::Texture();
    texture->type = VK_IMAGE_TYPE_3D;
    texture->width = width;
    texture->height = height;
    texture->depth = depth;
    texture->format = toFormat(format);
    texture->num_mipmaps = num_mipmaps;
    texture->num_layers = 1;
    texture->samples = VK_SAMPLE_COUNT_1_BIT;
    texture->tiling = VK_IMAGE_TILING_OPTIMAL;
    texture->flags = 0;

    createTextureData(device, texture, format, data, num_data_mipmaps, 1);
    return texture;
}

Texture *VulkanDriver::createTextureCube(uint32_t width,
                                         uint32_t height,
                                         uint32_t num_mipmaps,
                                         Format format,
                                         const void *data,
                                         uint32_t num_data_mipmaps)
{
    vulkan::Texture *texture = new vulkan::Texture();
    texture->type = VK_IMAGE_TYPE_2D;
    texture->width = width;
    texture->height = height;
    texture->depth = 1;
    texture->format = toFormat(format);
    texture->num_mipmaps = num_mipmaps;
    texture->num_layers = 6;
    texture->samples = VK_SAMPLE_COUNT_1_BIT;
    texture->tiling = VK_IMAGE_TILING_OPTIMAL;
    texture->flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    createTextureData(device, texture, format, data, num_data_mipmaps, 1);
    return texture;
}

FrameBuffer *VulkanDriver::createFrameBuffer(uint8_t num_attachments,
                                             const FrameBufferAttachment *attachments)
{
    // TODO: check for equal sizes (color + depthstencil)

    vulkan::FrameBuffer *result = new vulkan::FrameBuffer();

    uint32_t width = 0;
    uint32_t height = 0;
    VulkanRenderPassBuilder builder;

    builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS);

    // add color attachments
    result->num_attachments = 0;

    VkSampleCountFlagBits sample = VK_SAMPLE_COUNT_1_BIT;
    for (uint8_t i = 0; i < num_attachments; ++i) {
        const FrameBufferAttachment &attachment = attachments[i];
        VkImageView view = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
        bool resolve = false;
        if (attachment.type == FrameBufferAttachmentType::COLOR) {
            const FrameBufferAttachment::Color &color = attachment.color;
            const vulkan::Texture *color_texture = static_cast<const vulkan::Texture *>(color.texture);
            VkImageAspectFlags flags = Utils::getImageAspectFlags(color_texture->format);

            view = Utils::createImageView(
                device,
                color_texture->image, color_texture->format,
                flags, VK_IMAGE_VIEW_TYPE_2D,
                color.base_mip, color.num_mips,
                color.base_layer, color.num_layers
            );

            width = std::max<int>(1, color_texture->width / (1 << color.base_mip));
            height = std::max<int>(1, color_texture->height / (1 << color.base_mip));
            format = color_texture->format;
            sample = color_texture->samples;
            resolve = color.resolve_attachment;
            if (color.resolve_attachment) {
                builder.addColorResolveAttachment(color_texture->format);
                builder.addColorResolveAttachmentReference(0, i);
            } else {
                builder.addColorAttachment(color_texture->format, color_texture->samples);
                builder.addColorAttachmentReference(0, i);
            }
        } else if (attachment.type == FrameBufferAttachmentType::DEPTH) {
            const FrameBufferAttachment::Depth &depth = attachment.depth;
            const vulkan::Texture *depth_texture = static_cast<const vulkan::Texture *>(depth.texture);
            VkImageAspectFlags flags = Utils::getImageAspectFlags(depth_texture->format);

            view = Utils::createImageView(
                device,
                depth_texture->image, depth_texture->format,
                flags, VK_IMAGE_VIEW_TYPE_2D
            );

            width = depth_texture->width;
            height = depth_texture->height;
            format = depth_texture->format;
            sample = depth_texture->samples;
            builder.addDepthStencilAttachment(depth_texture->format, depth_texture->samples);
            builder.setDepthStencilAttachmentReference(0, i);
        } else if (attachment.type == FrameBufferAttachmentType::SWAP_CHAIN_COLOR) {
            const FrameBufferAttachment::SwapChainColor &swap_chain_color = attachment.swap_chain_color;
            const vulkan::SwapChain *swap_chain = static_cast<const vulkan::SwapChain *>(swap_chain_color.swap_chain);
            VkImageAspectFlags flags = Utils::getImageAspectFlags(swap_chain->surface_format.format);

            view = Utils::createImageView(
                device,
                swap_chain->images[swap_chain_color.base_image], swap_chain->surface_format.format,
                flags, VK_IMAGE_VIEW_TYPE_2D
            );

            width = swap_chain->sizes.width;
            height = swap_chain->sizes.height;
            format = swap_chain->surface_format.format;
            resolve = swap_chain_color.resolve_attachment;

            if (swap_chain_color.resolve_attachment) {
                builder.addColorResolveAttachment(swap_chain->surface_format.format,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                builder.addColorResolveAttachmentReference(0, i);
            } else {
                builder.addColorAttachment(swap_chain->surface_format.format, VK_SAMPLE_COUNT_1_BIT,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                builder.addColorAttachmentReference(0, i);
            }
        }

        result->attachments[result->num_attachments] = view;
        result->attachment_types[result->num_attachments] = attachment.type;
        result->attachment_format[result->num_attachments] = format;
        result->attachment_samples[result->num_attachments] = sample;
        result->attachment_resolve[result->num_attachments] = resolve;
        result->num_attachments++;
    }

    // create dummy renderpass
    result->dummy_render_pass = builder.build(device->LogicDevice()); // TODO: move to render pass cache
    result->sizes.width = width;
    result->sizes.height = height;

    // create framebuffer
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = result->dummy_render_pass;
    framebufferInfo.attachmentCount = result->num_attachments;
    framebufferInfo.pAttachments = result->attachments;
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device->LogicDevice(), &framebufferInfo, nullptr, &result->framebuffer) != VK_SUCCESS) {
        // TODO: log error
        delete result;
        result = nullptr;
    }

    return result;
}

CommandBuffer *VulkanDriver::createCommandBuffer(CommandBufferType type)
{

    // Create command buffers
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = device->CommandPool();
    allocateInfo.level =Utils::getCommandBufferLevel(type);
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device->LogicDevice(), &allocateInfo, &command_buffer) != VK_SUCCESS) {
        return nullptr;
    }
    auto vk_command_buffer = new vulkan::CommandBuffer;
    vk_command_buffer->command_buffer = command_buffer;
    vk_command_buffer->level = allocateInfo.level;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateSemaphore(device->LogicDevice(), &semaphoreInfo, nullptr, &vk_command_buffer->rendering_finished_gpu)
        != VK_SUCCESS ||
        vkCreateFence(device->LogicDevice(), &fenceInfo, nullptr, &vk_command_buffer->rendering_finished_cpu)
            != VK_SUCCESS
        ) {
        /// todo log
        return nullptr;
    }

    return vk_command_buffer;
}
UniformBuffer *VulkanDriver::createUniformBuffer(BufferType type, uint32_t size, const void *data)
{
    assert(type == BufferType::DYNAMIC && "Only dynamic buffers are implemented at the moment");
    assert(size != 0 && "Invalid size");

    vulkan::UniformBuffer *result = new vulkan::UniformBuffer();
    result->size = size;
    result->type = type;

    Utils::createBuffer(
        device,
        size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        result->buffer,
        result->memory
    );

    if (data != nullptr)
        Utils::fillHostVisibleBuffer(device,result->memory,size,data);

    return result;
}
Shader *VulkanDriver::createShaderFromSource(ShaderType type, uint32_t size, const char *data,
                                             const char *path)
{
    // convert GLSL/HLSL code to SPIR-V bytecode
    shaderc_compiler_t compiler = shaderc_compiler_initialize();
    shaderc_compile_options_t options = shaderc_compile_options_initialize();

    // set compile options
    shaderc_compile_options_set_include_callbacks(options,
                                                  shaderc::includeResolver,
                                                  shaderc::includeResultReleaser,
                                                  nullptr);

    // compile shader
    shaderc_compilation_result_t compilation_result = shaderc_compile_into_spv(
        compiler,
        data, size,
        shaderc_glsl_infer_from_source,
        path,
        "main",
        options
    );

    if (shaderc_result_get_compilation_status(compilation_result) != shaderc_compilation_status_success) {
        std::cerr << "VulkanDriver::createShaderFromSource(): can't compile shader at \"" << path << "\"" << std::endl;
        std::cerr << "\t" << shaderc_result_get_error_message(compilation_result);

        shaderc_result_release(compilation_result);
        shaderc_compile_options_release(options);
        shaderc_compiler_release(compiler);

        return nullptr;
    }

    size_t bytecode_size = shaderc_result_get_length(compilation_result);
    const uint32_t *bytecode_data = reinterpret_cast<const uint32_t *>(shaderc_result_get_bytes(compilation_result));

    vulkan::Shader *result = new vulkan::Shader();
    result->type = type;
    result->shaderModule = Utils::createShaderModule(device, bytecode_data, bytecode_size);

    shaderc_result_release(compilation_result);
    shaderc_compile_options_release(options);
    shaderc_compiler_release(compiler);

    return result;
}
Shader *VulkanDriver::createShaderFromBytecode(ShaderType type, uint32_t size, const void *data)
{
    assert(size != 0 && "Invalid size");
    assert(data != nullptr && "Invalid data");

    vulkan::Shader *result = new vulkan::Shader();
    result->type = type;
    result->shaderModule = Utils::createShaderModule(device, reinterpret_cast<const uint32_t *>(data), size);

    return result;
}
void VulkanDriver::destroyVertexBuffer(render::backend::VertexBuffer *vertex_buffer)
{
    if (vertex_buffer == nullptr)
        return;

    vulkan::VertexBuffer *vk_vertex_buffer = static_cast<vulkan::VertexBuffer *>(vertex_buffer);

    vmaDestroyBuffer(device->getVRAMAllocator(), vk_vertex_buffer->buffer, vk_vertex_buffer->memory);

    vk_vertex_buffer->buffer = VK_NULL_HANDLE;
    vk_vertex_buffer->memory = VK_NULL_HANDLE;

    delete vertex_buffer;
    vertex_buffer = nullptr;
}
void VulkanDriver::destroyIndexBuffer(render::backend::IndexBuffer *index_buffer)
{
    if (index_buffer == nullptr)
        return;

    vulkan::IndexBuffer *vk_index_buffer = static_cast<vulkan::IndexBuffer *>(index_buffer);

    vmaDestroyBuffer(device->getVRAMAllocator(), vk_index_buffer->buffer, vk_index_buffer->memory);


    vk_index_buffer->buffer = VK_NULL_HANDLE;
    vk_index_buffer->memory = VK_NULL_HANDLE;

    delete index_buffer;
    index_buffer = nullptr;
}

void *VulkanDriver::map(backend::VertexBuffer *vertex_buffer)
{
    assert(vertex_buffer != nullptr && "Invalid buffer");

    VertexBuffer *vk_vertex_buffer = static_cast<VertexBuffer *>(vertex_buffer);
    assert(vk_vertex_buffer->type == BufferType::DYNAMIC && "Mapped buffer must have BufferType::DYNAMIC type");

    void *result = nullptr;
    if (vmaMapMemory(device->getVRAMAllocator(), vk_vertex_buffer->memory, &result) != VK_SUCCESS)
    {
        // TODO: log error
    }

    return result;
}

void VulkanDriver::unmap(backend::VertexBuffer *vertex_buffer)
{
    assert(vertex_buffer != nullptr && "Invalid buffer");

    VertexBuffer *vk_vertex_buffer = static_cast<VertexBuffer *>(vertex_buffer);
    assert(vk_vertex_buffer->type == BufferType::DYNAMIC && "Mapped buffer must have BufferType::DYNAMIC type");

    vmaUnmapMemory(device->getVRAMAllocator(), vk_vertex_buffer->memory);
}

void *VulkanDriver::map(backend::IndexBuffer *index_buffer)
{
    assert(index_buffer != nullptr && "Invalid uniform buffer");

    IndexBuffer *vk_index_buffer = static_cast<IndexBuffer *>(index_buffer);
    assert(vk_index_buffer->type == BufferType::DYNAMIC && "Mapped buffer must have BufferType::DYNAMIC type");

    void *result = nullptr;
    if (vmaMapMemory(device->getVRAMAllocator(), vk_index_buffer->memory, &result) != VK_SUCCESS)
    {
        // TODO: log error
    }

    return result;
}

void VulkanDriver::unmap(backend::IndexBuffer *index_buffer)
{
    assert(index_buffer != nullptr && "Invalid buffer");

    IndexBuffer *vk_index_buffer = static_cast<IndexBuffer *>(index_buffer);
    assert(vk_index_buffer->type == BufferType::DYNAMIC && "Mapped buffer must have BufferType::DYNAMIC type");

    vmaUnmapMemory(device->getVRAMAllocator(), vk_index_buffer->memory);
}

void *VulkanDriver::map(render::backend::UniformBuffer *uniform_buffer)
{
    assert(uniform_buffer != nullptr && "Invalid uniform buffer");
    // TODO: check DYNAMIC buffer type

    vulkan::UniformBuffer *vk_uniform_buffer = static_cast<vulkan::UniformBuffer *>(uniform_buffer);

    void *result = nullptr;
    if (vmaMapMemory(device->getVRAMAllocator(), vk_uniform_buffer->memory, &result) != VK_SUCCESS)
    {
        // TODO: log error
    }
    return result;
}

void VulkanDriver::unmap(render::backend::UniformBuffer *uniform_buffer)
{
    assert(uniform_buffer != nullptr && "Invalid buffer");

    UniformBuffer *vk_uniform_buffer = static_cast<UniformBuffer *>(uniform_buffer);
    assert(vk_uniform_buffer->type == BufferType::DYNAMIC && "Mapped buffer must have BufferType::DYNAMIC type");

    vmaUnmapMemory(device->getVRAMAllocator(), vk_uniform_buffer->memory);

}


void VulkanDriver::destroyTexture(render::backend::Texture *texture)
{
    if (texture == nullptr)
        return;

    vulkan::Texture *vk_texture = static_cast<vulkan::Texture *>(texture);

    vmaDestroyImage(device->getVRAMAllocator(), vk_texture->image, vk_texture->memory);

    vk_texture->image = VK_NULL_HANDLE;
    vk_texture->memory = VK_NULL_HANDLE;
    vk_texture->format = VK_FORMAT_UNDEFINED;

    delete texture;
    texture = nullptr;
}
void VulkanDriver::destroyFrameBuffer(render::backend::FrameBuffer *frame_buffer)
{
    if (frame_buffer == nullptr)
        return;

    vulkan::FrameBuffer *vk_frame_buffer = static_cast<vulkan::FrameBuffer *>(frame_buffer);

    for (uint8_t i = 0; i < vk_frame_buffer->num_attachments; ++i) {
        vkDestroyImageView(device->LogicDevice(), vk_frame_buffer->attachments[i], nullptr);
        vk_frame_buffer->attachments[i] = VK_NULL_HANDLE;
    }

    vkDestroyFramebuffer(device->LogicDevice(), vk_frame_buffer->framebuffer, nullptr);
    vk_frame_buffer->framebuffer = VK_NULL_HANDLE;

    vkDestroyRenderPass(device->LogicDevice(), vk_frame_buffer->dummy_render_pass, nullptr);
    vk_frame_buffer->dummy_render_pass = VK_NULL_HANDLE;

    delete frame_buffer;
    frame_buffer = nullptr;
}
void VulkanDriver::destroyUniformBuffer(render::backend::UniformBuffer *uniform_buffer)
{
    if (uniform_buffer == nullptr)
        return;

    vulkan::UniformBuffer *vk_uniform_buffer = static_cast<vulkan::UniformBuffer *>(uniform_buffer);

    vmaDestroyBuffer(device->getVRAMAllocator(),vk_uniform_buffer->buffer,vk_uniform_buffer->memory);

    vk_uniform_buffer->buffer = VK_NULL_HANDLE;
    vk_uniform_buffer->memory = VK_NULL_HANDLE;

    delete uniform_buffer;
    uniform_buffer = nullptr;
}
void VulkanDriver::destroyShader(render::backend::Shader *shader)
{
    if (shader == nullptr)
        return;

    vulkan::Shader *vk_shader = static_cast<vulkan::Shader *>(shader);

    vkDestroyShaderModule(device->LogicDevice(), vk_shader->shaderModule, nullptr);
    vk_shader->shaderModule = VK_NULL_HANDLE;

    delete shader;
    shader = nullptr;
}

bool VulkanDriver::resetCommandBuffer(render::backend::CommandBuffer *command_buffer)
{

    if (command_buffer == nullptr) {
        return false;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer)->command_buffer;
    if (vkResetCommandBuffer(vk_command_buffer, 0) != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool VulkanDriver::beginCommandBuffer(render::backend::CommandBuffer *command_buffer)
{
    if (command_buffer == nullptr) {
        return false;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer)->command_buffer;
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    if (vkBeginCommandBuffer(vk_command_buffer, &beginInfo) != VK_SUCCESS) {
        return false;
    }
    return true;
}

bool VulkanDriver::endCommandBuffer(render::backend::CommandBuffer *command_buffer)
{
    if (command_buffer == nullptr) {
        return false;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer)->command_buffer;
    if (vkEndCommandBuffer(vk_command_buffer) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void VulkanDriver::beginRenderPass(
    render::backend::CommandBuffer *command_buffer,
    const render::backend::FrameBuffer *frame_buffer,
    const RenderPassInfo *info)
{
    if (command_buffer == nullptr) {
        return;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer);
    auto vk_frame_buffer = static_cast<const vulkan::FrameBuffer *>(frame_buffer);

    VkRenderPass render_pass = render_pass_cache->fetch(vk_frame_buffer, info);
    vk_context->setRenderPass(render_pass);
    vk_context->setFrameBuffer(vk_frame_buffer);

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = vk_frame_buffer->framebuffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = vk_frame_buffer->sizes;

    render_pass_info.clearValueCount = vk_frame_buffer->num_attachments;
    render_pass_info.pClearValues = reinterpret_cast<VkClearValue *>(info->clear_value);

    vkCmdBeginRenderPass(vk_command_buffer->command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

}
void VulkanDriver::endRenderPass(render::backend::CommandBuffer *command_buffer)
{
    if (command_buffer == nullptr) {
        return;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer)->command_buffer;
    vkCmdEndRenderPass(vk_command_buffer);
    vk_context->setRenderPass(VK_NULL_HANDLE);
}

void VulkanDriver::bindUniformBuffer(render::backend::BindSet *bind_set,
                                     uint32_t binding,
                                     const render::backend::UniformBuffer *uniform_buffer)
{
    assert(binding < vulkan::BindSet::MAX_BINDINGS);
    if (bind_set == nullptr) {
        return;
    }
    auto vk_bind_set = static_cast<vulkan::BindSet *>(bind_set);
    auto vk_ubo = static_cast<const vulkan::UniformBuffer *>(uniform_buffer);
    vk_bind_set->binding_used[binding] = true;
    vk_bind_set->binding_dirty[binding] = true;

    auto &data = vk_bind_set->binding_data[binding];
    auto &info = vk_bind_set->bindings[binding];
    if (vk_ubo == nullptr) {
        return;
    }

    data.ubo.buffer = vk_ubo->buffer;;
    data.ubo.size = vk_ubo->size;
    data.ubo.offset = 0;

    info.binding = binding;
    info.descriptorCount = 1;
    info.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;//todo change to
    info.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    info.pImmutableSamplers = nullptr;

}

void VulkanDriver::bindTexture(render::backend::BindSet *bind_set,
                               uint32_t binding,
                               const render::backend::Texture *texture)
{
    assert(binding < vulkan::BindSet::MAX_BINDINGS);

    if (bind_set == nullptr)
        return;

    vulkan::BindSet *vk_bind_set = static_cast<vulkan::BindSet *>(bind_set);
    const vulkan::Texture *vk_texture = static_cast<const vulkan::Texture *>(texture);
    uint32_t num_layers = (vk_texture) ? vk_texture->num_layers  : 1;
    uint32_t num_mipmaps = (vk_texture) ? vk_texture->num_mipmaps  : 1;
    bindTexture(bind_set, binding, texture, 0, num_mipmaps, 0, num_layers);
}

void VulkanDriver::bindTexture(render::backend::BindSet *bind_set,
                               uint32_t binding,
                               const render::backend::Texture *texture,
                               int base_mip,
                               int num_mip,
                               int base_layer,
                               int num_layer)
{
    assert(binding < vulkan::BindSet::MAX_BINDINGS);
    if (bind_set == nullptr) {
        return;
    }

    auto vk_bind_set = static_cast<vulkan::BindSet *>(bind_set);
    auto vk_texture = static_cast<const vulkan::Texture *>(texture);

    auto &data = vk_bind_set->binding_data[binding];
    auto &info = vk_bind_set->bindings[binding];

    if (vk_bind_set->binding_used[binding] &&
        info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        vkDestroyImageView(device->LogicDevice(), data.texture.view, nullptr);
        info = {};
        data = {};
    }

    VkImageView view{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
    if (vk_texture) {
        view = Utils::createImageView(device,
                                    vk_texture->image,
                                    vk_texture->format,
                                    Utils::getImageAspectFlags(vk_texture->format),
                                    Utils::getImageBaseViewType(vk_texture->type,vk_texture->flags,vk_texture->num_layers),
                                    base_mip, num_mip,
                                    base_layer, num_layer);
        sampler = vk_texture->sampler;
    }

    vk_bind_set->binding_used[binding] = true;
    vk_bind_set->binding_dirty[binding] = true;
    data.texture.sampler = sampler;
    data.texture.view = view;
    info.binding = binding;
    info.descriptorCount = 1;
    info.stageFlags = VK_SHADER_STAGE_ALL;//todo change to
    info.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    info.pImmutableSamplers = nullptr;
}

void VulkanDriver::drawIndexedPrimitive(
    render::backend::CommandBuffer* command_buffer,
    const render::backend::RenderPrimitive *render_primitive)
{
    if (command_buffer == nullptr) {
        return;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer);
    auto vk_vertex_buffer = static_cast<const vulkan::VertexBuffer *>(render_primitive->vertex_buffer);
    auto vk_index_buffer = static_cast<const vulkan::IndexBuffer*>(render_primitive->index_buffer);


    std::vector<VkDescriptorSet> sets(vk_context->getNumBindSets());
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorImageInfo> image_infos;
    std::vector<VkDescriptorBufferInfo> buffer_infos;

    writes.reserve(vulkan::BindSet::MAX_BINDINGS * vk_context->getNumBindSets());
    image_infos.reserve(vulkan::BindSet::MAX_BINDINGS * vk_context->getNumBindSets());
    buffer_infos.reserve(vulkan::BindSet::MAX_BINDINGS * vk_context->getNumBindSets());

    for (uint8_t i = 0; i < vk_context->getNumBindSets(); ++i) {
        auto *bind_set = vk_context->getBindSet(i);

        VkDescriptorSetLayout new_layout = descriptor_set_layout_cache->fetch(bind_set);
        updateBindSetLayout(device, bind_set, new_layout);

        sets[i] = bind_set->set;
        for (uint8_t j = 0; j < vulkan::BindSet::MAX_BINDINGS; ++j) {
            if (!bind_set->binding_used[j])
                continue;

            if (!bind_set->binding_dirty[j])
                continue;

            VkDescriptorType descriptor_type = bind_set->bindings[j].descriptorType;
            auto &data = bind_set->binding_data[j];

            VkWriteDescriptorSet write_set = {};
            write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_set.dstSet =  bind_set->set;
            write_set.dstBinding = j;
            write_set.dstArrayElement = 0;

            switch (descriptor_type)
            {
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                {
                    VkDescriptorImageInfo info = {};
                    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    info.imageView = data.texture.view;
                    info.sampler = data.texture.sampler;

                    image_infos.push_back(info);
                    write_set.pImageInfo = &image_infos[image_infos.size() - 1];
                }break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                {
                    VkDescriptorBufferInfo info = {};
                    info.buffer = data.ubo.buffer;
                    info.offset = data.ubo.offset;
                    info.range = data.ubo.size;

                    buffer_infos.push_back(info);
                    write_set.pBufferInfo = &buffer_infos[buffer_infos.size() - 1];
                }break;
                default:
                {
                    assert(false && "Unsupported descriptor type");
                }break;
            }
            write_set.descriptorType = descriptor_type;
            write_set.descriptorCount = 1;

            writes.push_back(write_set);

            bind_set->binding_dirty[j] = false;
        }
    }

    if (writes.size() > 0)
        vkUpdateDescriptorSets(device->LogicDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

    VkPipelineLayout pipeline_layout = pipeline_layout_cache->fetch(vk_context);
    VkPipeline pipeline = pipeline_cache->fetch(vk_context, render_primitive);
    vkCmdBindPipeline(vk_command_buffer->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    if (vk_context->getPushConstantsSize() > 0) {
        vkCmdPushConstants(vk_command_buffer->command_buffer,pipeline_layout,VK_SHADER_STAGE_ALL,0,vk_context->getPushConstantsSize(),
                           vk_context->getPushConstants());
    }

    if (sets.size() > 0){
        vkCmdBindDescriptorSets(vk_command_buffer->command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline_layout,
                                0,
                                static_cast<uint32_t>(sets.size()),
                                sets.data(),
                                0,
                                nullptr);
    }

    VkViewport viewport = vk_context->getViewport();
    VkRect2D scissor = vk_context->getScissor();

    vkCmdSetViewport(vk_command_buffer->command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(vk_command_buffer->command_buffer, 0, 1, &scissor);

    VkBuffer vertex_buffers[] = {vk_vertex_buffer->buffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(vk_command_buffer->command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(vk_command_buffer->command_buffer,
                         vk_index_buffer->buffer,
                         0,
                         vk_index_buffer->index_type);

    uint32_t num_instance = 1;
    uint32_t base_instance = 0;

    vkCmdDrawIndexed(vk_command_buffer->command_buffer, render_primitive->num_indices,
                     num_instance, render_primitive->base_index, render_primitive->vertex_base_offset, base_instance);

}
void VulkanDriver::drawIndexedPrimitiveInstanced(render::backend::CommandBuffer *command_buffer,
                                                 const render::backend::RenderPrimitive *primitive,
                                                 const render::backend::VertexBuffer *instance_buffer,
                                                 uint32_t num_instances,
                                                 uint32_t offset)
{
    if (command_buffer == nullptr) {
        return;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer)->command_buffer;

}

SwapChain *VulkanDriver::createSwapChain(void *native_window, uint32_t width, uint32_t height)
{
    assert(native_window != nullptr && "native_window nullptr");
    vulkan::SwapChain *swapchain = new vulkan::SwapChain;

    //todo platform::createSurface()
    VK_CHECK(glfwCreateWindowSurface(device->Instance(), (GLFWwindow *) native_window,
                                     nullptr, &swapchain->surface), "Create Surface failed");

    // get present queue family
    swapchain->present_queue_family = Utils::getPresentQueueFamily(
        device->PhysicalDevice(),
        swapchain->surface,
        device->GraphicsQueueFamily()
    );

    // Get present queue
    vkGetDeviceQueue(device->LogicDevice(), swapchain->present_queue_family, 0, &swapchain->present_queue);
    if (swapchain->present_queue == VK_NULL_HANDLE) {
        std::cerr << "Can't get present queue from logical device" << std::endl;
        return nullptr;
    }

    // select swapchain settings
    vulkan::selectOptimalSwapChainSettings(device, swapchain);

    //Create Sync Object
    createSwapchainObjects(device, swapchain, width, height);

    return swapchain;
}

void VulkanDriver::destroySwapChain(render::backend::SwapChain *swapchain)
{
    if (swapchain == nullptr)
        return;

    auto *vk_swap_chain = static_cast<vulkan::SwapChain *>(swapchain);

    //Destroy Sync Object
    vk_swap_chain->present_queue_family = 0xFFFF;
    vk_swap_chain->present_queue = VK_NULL_HANDLE;
    vk_swap_chain->present_mode = VK_PRESENT_MODE_FIFO_KHR;
    vk_swap_chain->sizes = {0, 0};

    vk_swap_chain->num_images = 0;
    vk_swap_chain->current_image = 0;

    destroySwapchainObjects(device, vk_swap_chain);

    // destroy Surface
    vulkan::platform::destroySurface(device, &vk_swap_chain->surface);
    vk_swap_chain->surface = VK_NULL_HANDLE;
    delete swapchain;
    swapchain = nullptr;
}

bool VulkanDriver::acquire(render::backend::SwapChain *swap_chain, uint32_t *image_index)
{
    vulkan::SwapChain *vk_swap_chain = static_cast<vulkan::SwapChain *>(swap_chain);
    uint32_t current_image = vk_swap_chain->current_image;

    VkResult result = vkAcquireNextImageKHR(
        device->LogicDevice(),
        vk_swap_chain->swap_chain,
        std::numeric_limits<uint64_t>::max(),
        vk_swap_chain->image_available_gpu[current_image],
        VK_NULL_HANDLE,
        image_index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        return false;

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        // TODO: log fatal
        // runtime_error("Can't acquire swap chain image");
        return false;
    }

    return true;
}

bool VulkanDriver::submit(render::backend::CommandBuffer *command_buffer)
{
    if (command_buffer == nullptr) {
        return false;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer);
    VkSubmitInfo submitInfo = {};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_command_buffer->command_buffer;
    //    submitInfo.signalSemaphoreCount = 1;
    //    submitInfo.pSignalSemaphores = &vk_command_buffer->rendering_finished_gpu;

    vkResetFences(device->LogicDevice(), 1, &vk_command_buffer->rendering_finished_cpu);
    if (vkQueueSubmit(device->GraphicsQueue(), 1, &submitInfo, vk_command_buffer->rendering_finished_cpu)
        != VK_SUCCESS)
        return false;
    return true;
}

bool VulkanDriver::submitSynced(render::backend::CommandBuffer *command_buffer,
                                render::backend::SwapChain *wait_swap_chain)
{
    if (command_buffer == nullptr) {
        return false;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer);
    VkSubmitInfo submitInfo = {};

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_command_buffer->command_buffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vk_command_buffer->rendering_finished_gpu;
    VkPipelineStageFlags waitStages=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (wait_swap_chain != nullptr) {
        auto vk_swap_chain = static_cast<vulkan::SwapChain *>(wait_swap_chain);
        uint32_t current_iamge = vk_swap_chain->current_image;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &vk_swap_chain->image_available_gpu[current_iamge];
        submitInfo.pWaitDstStageMask =&waitStages;
    }
    vkResetFences(device->LogicDevice(), 1, &vk_command_buffer->rendering_finished_cpu);
    if (vkQueueSubmit(device->GraphicsQueue(), 1, &submitInfo, vk_command_buffer->rendering_finished_cpu)
        != VK_SUCCESS)
        return false;
    return true;
}

bool VulkanDriver::submitSynced(render::backend::CommandBuffer *command_buffer,
                                uint32_t num_wait_command_buffers,
                                const render::backend::CommandBuffer *wait_command_buffers)
{
    if (command_buffer == nullptr) {
        return false;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vk_command_buffer->command_buffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vk_command_buffer->rendering_finished_gpu;

    std::vector<VkSemaphore> wait_semaphores;
    std::vector<VkPipelineStageFlags> wait_stages;
    if (num_wait_command_buffers != 0 && wait_command_buffers != nullptr) {
        auto vk_command_buffer = static_cast<const vulkan::CommandBuffer *>(wait_command_buffers);
        wait_semaphores.resize(num_wait_command_buffers);
        wait_stages.resize(num_wait_command_buffers);

        for (int i = 0; i < num_wait_command_buffers; ++i) {
            wait_semaphores[i] = vk_command_buffer[i].rendering_finished_gpu;
            wait_stages[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        submitInfo.waitSemaphoreCount = num_wait_command_buffers;
        submitInfo.pWaitSemaphores = wait_semaphores.data();
        submitInfo.pWaitDstStageMask = wait_stages.data();

    }
    vkResetFences(device->LogicDevice(), 1, &vk_command_buffer->rendering_finished_cpu);
    if (vkQueueSubmit(device->GraphicsQueue(), 1, &submitInfo, vk_command_buffer->rendering_finished_cpu)
        != VK_SUCCESS)
        return false;
    return true;
}

bool VulkanDriver::present(render::backend::SwapChain *swap_chain,
                           uint32_t num_wait_command_buffers,
                           const render::backend::CommandBuffer *wait_command_buffers)
{
    vulkan::SwapChain *vk_swap_chain = static_cast<vulkan::SwapChain *>(swap_chain);
    uint32_t current_image = vk_swap_chain->current_image;

    std::vector<VkSemaphore> wait_semaphores(num_wait_command_buffers);
    std::vector<VkFence> wait_fences(num_wait_command_buffers);
    if (num_wait_command_buffers != 0 && wait_command_buffers != nullptr) {
        auto vk_command_buffer = static_cast<const vulkan::CommandBuffer *>(wait_command_buffers);

        for (int i = 0; i < num_wait_command_buffers; ++i) {
            wait_semaphores[i] = vk_command_buffer[i].rendering_finished_gpu;
            wait_fences[i] = vk_command_buffer[i].rendering_finished_cpu;
        }
    }

    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.swapchainCount = 1;
    info.pSwapchains = &vk_swap_chain->swap_chain;
    info.pImageIndices = &current_image;

    if (wait_semaphores.size()) {
        info.waitSemaphoreCount = num_wait_command_buffers;
        info.pWaitSemaphores = wait_semaphores.data();
    }

    Utils::transitionImageLayout(
        device,
        vk_swap_chain->images[current_image],
        vk_swap_chain->surface_format.format,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );

    VkResult result = vkQueuePresentKHR(vk_swap_chain->present_queue, &info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        return false;

    if (result != VK_SUCCESS) {
        // TODO: log fatal
        return false;
    }

    if (wait_fences.size()) {
        vkWaitForFences(
            device->LogicDevice(),
            wait_fences.size(), wait_fences.data(),
            VK_TRUE, std::numeric_limits<uint64_t>::max()
        );
    }

    vk_swap_chain->current_image++;
    vk_swap_chain->current_image %= vk_swap_chain->num_images;

    return true;
}

void VulkanDriver::wait()
{
    assert(device != nullptr);
    device->wait();
}


void VulkanDriver::generateTexture2DMipmaps(render::backend::Texture *texture)
{
    assert(texture != nullptr && "Invalid texture");

    vulkan::Texture *vk_texture = static_cast<vulkan::Texture *>(texture);

    // prepare for transfer
    Utils::transitionImageLayout(
        device,
        vk_texture->image,
        vk_texture->format,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        vk_texture->num_mipmaps
    );

    Utils::generateImage2DMipmaps(
        device,
        vk_texture->image,
        vk_texture->format,
        vk_texture->width,
        vk_texture->height,
        vk_texture->num_mipmaps,
        vk_texture->format,
        VK_FILTER_LINEAR
    );

    // prepare for shader access
    Utils::transitionImageLayout(
        device,
        vk_texture->image,
        vk_texture->format,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0,
        vk_texture->num_mipmaps
    );
}
Multisample VulkanDriver::getMaxSampleCount()
{
    assert(device != nullptr && "Invalid context");

    VkSampleCountFlagBits samples = device->getMaxSampleCount();
    return Utils::getApiSamples(samples);
}

Format VulkanDriver::getOptimalDepthFormat()
{
    assert(device != nullptr && "Invalid context");

    VkFormat format = Utils::selectOptimalDepthFormat(device->PhysicalDevice());
    return Utils::getApiFormat(format);
}

VkSampleCountFlagBits VulkanDriver::toMultisample(Multisample samples)
{
    return Utils::getSamples(samples);
}

Multisample VulkanDriver::fromMultisample(VkSampleCountFlagBits samples)
{
    return Utils::getApiSamples(samples);
}

VkFormat VulkanDriver::toFormat(Format format)
{
    return Utils::getFormat(format);
}

Format VulkanDriver::fromFormat(VkFormat format)
{
    return Utils::getApiFormat(format);
}

void VulkanDriver::destroyCommandBuffer(render::backend::CommandBuffer *command_buffer)
{
    if (!command_buffer) {
        return;
    }
    auto vk_command_buffer = static_cast<vulkan::CommandBuffer *>(command_buffer);
    vkFreeCommandBuffers(device->LogicDevice(), device->CommandPool(), 1, &vk_command_buffer->command_buffer);
    vk_command_buffer->command_buffer = VK_NULL_HANDLE;
    vk_command_buffer->level = 0;
    vkDestroySemaphore(device->LogicDevice(), vk_command_buffer->rendering_finished_gpu, nullptr);
    vk_command_buffer->rendering_finished_gpu = VK_NULL_HANDLE;
    vkDestroyFence(device->LogicDevice(), vk_command_buffer->rendering_finished_cpu, nullptr);
    vk_command_buffer->rendering_finished_cpu = VK_NULL_HANDLE;
    delete command_buffer;
    command_buffer = nullptr;
}

void VulkanDriver::clearShaders()
{
    vk_context->clearShaders();
}

void VulkanDriver::clearBindSets()
{
    vk_context->clearBindSets();
}

void VulkanDriver::setShader(ShaderType type, const render::backend::Shader *shader)
{
    auto vk_shader = static_cast<const vulkan::Shader *>(shader);
    vk_context->setShader(type, vk_shader);
}

void VulkanDriver::pushBindSet(render::backend::BindSet *set)
{
    auto vk_bind_set = static_cast<vulkan::BindSet *>(set);
    vk_context->pushBindSet(vk_bind_set);
}

void VulkanDriver::setBindSet(uint32_t binding,const render::backend::BindSet *set)
{
    auto * vk_bind_set = static_cast<const vulkan::BindSet *>(set);
    vk_context->setBindSet(binding, vk_bind_set);
}

BindSet *VulkanDriver::createBindSet()
{
    auto result = new render::backend::vulkan::BindSet;
    memset(result, 0, sizeof(vulkan::BindSet));
    return result;
}

void VulkanDriver::destroyBindSet(render::backend::BindSet *set)
{
    if (!set) {
        auto vk_bind_set = static_cast<vulkan::BindSet *>(set);
        for (int i = 0; i < render::backend::vulkan::BindSet::MAX_BINDINGS; ++i) {
            if (!vk_bind_set->binding_used[i]) {
                continue;
            }

            auto &info = vk_bind_set->bindings[i];
            if (info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                continue;
            }
            auto &data = vk_bind_set->binding_data[i];
            vkDestroyImageView(device->LogicDevice(), data.texture.view, nullptr);
        }

        if (vk_bind_set->set != VK_NULL_HANDLE){
            vkFreeDescriptorSets(device->LogicDevice(), device->DescriptorPool(), 1, &vk_bind_set->set);
            vk_bind_set->set =VK_NULL_HANDLE;
        }


        delete vk_bind_set;
        set = nullptr;

    }
}

/// render state
void VulkanDriver::setCullMode(CullMode cull_mode)
{
    VkCullModeFlags mode = Utils::getCullMode(cull_mode);
    vk_context->setCullMode(mode);
}

void VulkanDriver::setDepthTest(bool enable)
{
    vk_context->setDepthTest(enable);
}

void VulkanDriver::setDepthWrite(bool enable)
{
    vk_context->setDepthWrite(enable);
}

void VulkanDriver::setDepthCompareFunc(DepthCompareFunc depth_compare_func)
{
    VkCompareOp depth_compare = Utils::getDepthCompareFunc(depth_compare_func);
    vk_context->setDepthCompareFunc(depth_compare);
}

void VulkanDriver::setBlending(bool enable)
{
    vk_context->setBlending(enable);
}

void VulkanDriver::setBlendFactor(BlendFactor src_factor, BlendFactor dst_factor)
{
    VkBlendFactor vk_src_factor = Utils::getBlendFactor(src_factor);
    VkBlendFactor vk_dst_factor = Utils::getBlendFactor(dst_factor);
    vk_context->setBlendFactor(vk_src_factor, vk_dst_factor);
}

bool VulkanDriver::wait(uint32_t num_wait_command_buffers, render::backend::CommandBuffer *const *wait_command_buffers)
{
    {
        if (num_wait_command_buffers == 0)
            return true;

        std::vector<VkFence> wait_fences(num_wait_command_buffers);

        for (uint32_t i = 0; i < num_wait_command_buffers; ++i) {
            const vulkan::CommandBuffer
                *vk_wait_command_buffer = static_cast<const vulkan::CommandBuffer *>(wait_command_buffers[i]);
            wait_fences[i] = vk_wait_command_buffer->rendering_finished_cpu;
        }

        VkResult result =
            vkWaitForFences(device->LogicDevice(), num_wait_command_buffers, wait_fences.data(), VK_TRUE, UINT64_MAX);
        return result == VK_SUCCESS;
    }
}

void VulkanDriver::clearPushConstants()
{
    vk_context->clearPushConstants();
}

void VulkanDriver::setPushConstant(uint8_t size, const void *data)
{
    vk_context->setPushConstant(size,data);
}

void VulkanDriver::allocateBindSets(uint8_t size)
{
    vk_context->allocateBindSets(size);
}

void VulkanDriver::setViewport(float x, float y, float width, float height)
{
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    vk_context->setViewport(viewport);
}

void VulkanDriver::setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    VkRect2D scissor;
    scissor.offset={x,y};
    scissor.extent={width,height};
    vk_context->setScissor(scissor);
}

}