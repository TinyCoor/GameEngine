//
// Created by 12132 on 2021/11/7.
//

#ifndef GAMEENGINE_VULKANIMGUIRENDER_H
#define GAMEENGINE_VULKANIMGUIRENDER_H

#include <volk.h>
#include <map>
#include "driver.h"
struct ImGuiContext;
struct ImDrawData;
typedef void* ImTextureID;
namespace render::backend::vulkan {

struct RenderState;

struct VulkanRenderFrame;
class VulkanSwapChain;
class VulkanTexture;
class Device;


class ImGuiRender {
public:
  ImGuiRender(render::backend::Driver *driver);

  virtual ~ImGuiRender();

  void init(ImGuiContext *imguiContext);

  void shutdown();

  void render(const VulkanRenderFrame &frame);

  ImTextureID fetchTextureID(const render::backend::Texture *texture);
  void invalidateTextureIDs();

private:
    void updateBuffers(const ImDrawData *draw_data);
    void setupRenderState(const vulkan::VulkanRenderFrame &frame, const ImDrawData *draw_data);

private:
  render::backend::Driver *driver{nullptr};
  render::backend::Texture* font_texture{nullptr};
  render::backend::Shader* vertex_shader{nullptr};
  render::backend::Shader* fragment_shader{nullptr};

  render::backend::VertexBuffer* vertices{nullptr};
  render::backend::IndexBuffer* indices{nullptr};
  size_t index_buffer_size{0};
  size_t vertex_buffer_size{0};

  render::backend::BindSet *font_bind_set {nullptr};
  std::map<const render::backend::Texture *, render::backend::BindSet *> registered_textures;


};

}
#endif //GAMEENGINE_VULKANIMGUIRENDER_H
