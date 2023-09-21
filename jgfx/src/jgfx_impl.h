#pragma once

#include "renderer_vk.h"
#include "jgfx/jgfx.h"

namespace jgfx {
  struct ContextImpl {
    bool init(const InitInfo& initInfo);
    void shutdown();

    ShaderHandle newShader();
    ProgramHandle newProgram();

  private:
    vk::RenderContextVK vkCtx;
  };
}
