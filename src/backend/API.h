//
// Created by 12132 on 2021/11/20.
//

#ifndef GAMEENGINE_API_H
#define GAMEENGINE_API_H

#include <cstdint>
//todo std::function;
namespace render::backend {
enum class Api {
    DEFAULT,
    VULKAN,
    OPENGL,
    DIRECTX,
    CPU,
    MAX,
};
enum BufferType : unsigned char {
    STATIC = 0,
    DYNAMIC,
    MAX,
};

//图元类型
enum class RenderPrimitiveType {
    POINTS = 0,
    LINES,
    LINE_STRIP,
    LINE_LOOP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_LOOP,
    QUADS,
    QUAD_STRIP,
    QUAD_LOOP,
    PATCHES,
    MAX,
};

enum class IndexSize {
    UINT16 = 0,
    UINT32,
    MAX,
};

enum class Multisample : unsigned char {
    COUNT_1 = 0,
    COUNT_2,
    COUNT_4,
    COUNT_8,
    COUNT_16,
    COUNT_32,
    COUNT_64,

    MAX,
};

enum class Format {
    UNDEFINED = 0,

    // 8-bit formats
    R8_UNORM,
    R8_SNORM,
    R8_UINT,
    R8_SINT,

    R8G8_UNORM,
    R8G8_SNORM,
    R8G8_UINT,
    R8G8_SINT,

    R8G8B8_UNORM,
    R8G8B8_SNORM,
    R8G8B8_UINT,
    R8G8B8_SINT,

    B8G8R8_UNORM,
    B8G8R8_SNORM,
    B8G8R8_UINT,
    B8G8R8_SINT,

    R8G8B8A8_UNORM,
    R8G8B8A8_SNORM,
    R8G8B8A8_UINT,
    R8G8B8A8_SINT,

    B8G8R8A8_UNORM,
    B8G8R8A8_SNORM,
    B8G8R8A8_UINT,
    B8G8R8A8_SINT,

    // 16-bit formats
    R16_UNORM,
    R16_SNORM,
    R16_UINT,
    R16_SINT,
    R16_SFLOAT,

    R16G16_UNORM,
    R16G16_SNORM,
    R16G16_UINT,
    R16G16_SINT,
    R16G16_SFLOAT,

    R16G16B16_UNORM,
    R16G16B16_SNORM,
    R16G16B16_UINT,
    R16G16B16_SINT,
    R16G16B16_SFLOAT,

    R16G16B16A16_UNORM,
    R16G16B16A16_SNORM,
    R16G16B16A16_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_SFLOAT,

    // 32-bit formats
    R32_UINT,
    R32_SINT,
    R32_SFLOAT,

    R32G32_UINT,
    R32G32_SINT,
    R32G32_SFLOAT,

    R32G32B32_UINT,
    R32G32B32_SINT,
    R32G32B32_SFLOAT,

    R32G32B32A32_UINT,
    R32G32B32A32_SINT,
    R32G32B32A32_SFLOAT,

    // depth formats
    D16_UNORM,
    D16_UNORM_S8_UINT,
    D24_UNORM_S8_UINT,
    D32_SFLOAT,
    D32_SFLOAT_S8_UINT,

    MAX,
};

enum class ShaderType {
    // Graphics pipeline
    VERTEX = 0,
    TESSELLATION_CONTROL,
    TESSELLATION_EVALUATION,
    GEOMETRY,
    FRAGMENT,

    // Compute pipeline
    COMPUTE,

    // Raytracing pipeline
    RAY_GENERATION,
    INTERSECTION,
    ANY_HIT,
    CLOSEST_HIT,
    MISS,
    CALLABLE,
};

enum class CommandBufferType : unsigned char {
    PRIMARY = 0,
    SECONDARY
};

struct VertexBuffer {

};

struct IndexBuffer {

};

struct RenderPrimitive {
};

struct Texture {};
struct SwapChain {};
struct FrameBuffer {};
struct CommandBuffer {};

enum FrameBufferAttachmentType : unsigned char {
    COLOR = 0,
    DEPTH,
    SWAP_CHAIN_COLOR,
};

struct FrameBufferAttachment {
    struct Color {
        const Texture *texture{nullptr};
        int base_mip{0};
        int num_mips{1};
        int base_layer{0};
        int num_layers{1};
        bool resolve_attachment{false};
    };

    struct Depth {
        const Texture *texture{nullptr};
    };

    struct SwapChainColor {
        const SwapChain *swap_chain{nullptr};
        int base_image{0};
        bool resolve_attachment{false};
    };

    FrameBufferAttachmentType type{FrameBufferAttachmentType::COLOR};
    union {
        Color color;
        Depth depth;
        SwapChainColor swap_chain_color;
    };
};

struct UniformBuffer {

};

struct Shader {

};
//todo shader storage buffer?

struct GraphicsProgram {

};

struct RenderTarget {

};

// C structs
struct VertexAttribute {
    Format format{Format::UNDEFINED};
    unsigned int offset{0};
};

// main backend class
class Driver {
public:
    virtual ~Driver() = default;
    static Driver *create(const char *app_name, const char *engine_name, Api api);
    virtual VertexBuffer *createVertexBuffer(
        BufferType type,
        uint16_t vertex_size,
        uint32_t num_vertices,
        uint8_t num_attributes,
        const VertexAttribute *attributes,
        const void *data
    ) = 0;

    virtual IndexBuffer *createIndexBuffer(
        BufferType type,
        IndexSize index_size,
        uint32_t num_indices,
        const void *data
    ) = 0;

    virtual RenderPrimitive *createRenderPrimitive(
        RenderPrimitiveType type,
        const VertexBuffer *vertex_buffer,
        const IndexBuffer *index_buffer
    ) = 0;

    virtual Texture *createTexture2D(
        uint32_t width,
        uint32_t height,
        uint32_t num_mipmaps,
        Format format,
        Multisample samples = Multisample::COUNT_1,
        const void *data = nullptr,
        uint32_t num_data_mipmaps = 1
    ) = 0;

    virtual Texture *createTexture2DArray(
        uint32_t width,
        uint32_t height,
        uint32_t num_mipmaps,
        uint32_t num_layers,
        Format format,
        const void *data = nullptr,
        uint32_t num_data_mipmaps = 1,
        uint32_t num_data_layers = 1
    ) = 0;

    virtual Texture *createTexture3D(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t num_mipmaps,
        Format format,
        const void *data = nullptr,
        uint32_t num_data_mipmaps = 1
    ) = 0;

    virtual Texture *createTextureCube(
        uint32_t width,
        uint32_t height,
        uint32_t num_mipmaps,
        Format format,
        const void *data = nullptr,
        uint32_t num_data_mipmaps = 1
    ) = 0;

    virtual FrameBuffer *createFrameBuffer(
        uint8_t num_attachments,
        const FrameBufferAttachment *attachments
    ) = 0;

    virtual CommandBuffer *createCommandBuffer(
        CommandBufferType type
    ) = 0;

    virtual UniformBuffer *createUniformBuffer(
        BufferType type,
        uint32_t size,
        const void *data = nullptr
    ) = 0;

    virtual Shader *createShaderFromSource(
        ShaderType type,
        uint32_t length,
        const char *data,
        const char *path
    ) = 0;

    virtual Shader *createShaderFromBytecode(
        ShaderType type,
        uint32_t size,
        const void *data
    ) = 0;

    virtual SwapChain *createSwapChain(void *native_window, uint32_t width, uint32_t height) = 0;
    virtual void destroyVertexBuffer(VertexBuffer *vertex_buffer) = 0;
    virtual void destroyIndexBuffer(IndexBuffer *index_buffer) = 0;
    virtual void destroyRenderPrimitive(RenderPrimitive *render_primitive) = 0;
    virtual void destroyTexture(Texture *texture) = 0;
    virtual void destroyFrameBuffer(FrameBuffer *frame_buffer) = 0;
    virtual void destroyCommandBuffer(CommandBuffer *command_buffer) = 0;
    virtual void destroyUniformBuffer(UniformBuffer *uniform_buffer) = 0;
    virtual void destroyShader(Shader *shader) = 0;
    virtual void destroySwapChain(SwapChain *swapchain) = 0;

public:
    virtual Multisample getMaxSampleCount() = 0;
    virtual Format getOptimalDepthFormat() = 0;

public:
    virtual void generateTexture2DMipmaps(Texture *texture) = 0;
    virtual void *map(render::backend::UniformBuffer *uniform_buffer) = 0;
    virtual void unmap(render::backend::UniformBuffer *uniform_buffer) = 0;
    virtual void wait() = 0;
    virtual bool acquire(SwapChain *swapchain, uint32_t *image_index) = 0;
    virtual bool present(SwapChain *swapchain,
                         uint32_t num_wait_command_buffers,
                         const CommandBuffer* wait_command_buffers) = 0;
    virtual bool submit(CommandBuffer* command_buffer) = 0;
    virtual bool submitSynced(CommandBuffer* command_buffer,
                              SwapChain* wait_swap_chain) = 0;
    virtual bool submitSynced(CommandBuffer* command_buffer,
                              uint32_t num_wait_command_buffers,
                              const CommandBuffer* wait_command_buffers) = 0;
public:
    virtual bool reset(
        CommandBuffer* command_buffer
        ) = 0;

    virtual bool begin(
        CommandBuffer *command_buffer) = 0;

    virtual bool end(
        CommandBuffer *command_buffer
    ) = 0;

    // render pass
    virtual void beginRenderPass(
        CommandBuffer* command_buffer,
        const FrameBuffer *frame_buffer
    ) = 0;

    virtual void endRenderPass(
        CommandBuffer* command_buffer
    ) = 0;

    // bind
    virtual void bindUniformBuffer(
        uint32_t unit,
        const UniformBuffer *uniform_buffer
    ) = 0;

    virtual void bindTexture(
        uint32_t unit,
        const Texture *texture
    ) = 0;

    virtual void bindShader(
        const Shader *shader
    ) = 0;

    // draw
    virtual void drawIndexedPrimitive(
        CommandBuffer* command_buffer,
        const RenderPrimitive *render_primitive
    ) = 0;

    virtual void drawIndexedPrimitiveInstanced(
        CommandBuffer* command_buffer,
        const RenderPrimitive *primitive,
        const VertexBuffer *instance_buffer,
        uint32_t num_instances,
        uint32_t offset
    ) = 0;
};

extern Driver *createDriver(const char *application_name, const char *engine_name, Api api = Api::DEFAULT);
extern void destroyDriver(Driver *driver);

}

#endif //GAMEENGINE_API_H
