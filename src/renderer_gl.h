#pragma once

#include "renderer.h"

namespace jgfx::gl {
  struct TextureGL {

  };

  struct FramebufferGL {

  };

  struct ShaderGL {

  };

  struct BufferGL {

  };

  struct ProgramGL {

  };

  struct RenderContextGL : public RenderContext {
    // Initialization
    bool init(const InitInfo& createInfo) override;
    void shutdown() override;

    ShaderGL _shaders[MAX_SHADERS];
    ProgramGL _programs[MAX_PROGRAMS];
    BufferGL _buffers[MAX_BUFFERS];
    TextureGL _textures[MAX_IMAGES];
    FramebufferGL _framebuffer[MAX_FRAMEBUFFERS];
  };
}