#pragma once

#include "renderer.h"

namespace jgfx::gl {
  struct TextureGL {
    bool create();
    void destroy();
  };

  struct FramebufferGL {
    bool create();
    void destroy();
  };

  struct ShaderGL {
    bool create();
    void destroy();
  };

  struct BufferGL {
    bool create();
    void destroy();
  };

  struct ProgramGL {
    bool create();
    void destroy();
  };

  struct RenderContextGL : public RenderContext {
    // Initialization
    bool init(const InitInfo& createInfo) override;
    void shutdown() override;

    // ObjectGL creation
    void newShader(ShaderHandle handle, const void* binData, uint32_t size);
    void newProgram(ProgramHandle handle);
    void newBuffer(BufferHandle handle, const void* data, uint32_t size, BufferType type);
    void newImage(ImageHandle handle, const void* data, uint32_t size, const TextureDesc& desc);

    ShaderGL _shaders[MAX_SHADERS];
    ProgramGL _programs[MAX_PROGRAMS];
    BufferGL _buffers[MAX_BUFFERS];
    TextureGL _textures[MAX_IMAGES];
    FramebufferGL _framebuffer[MAX_FRAMEBUFFERS];
  };
}