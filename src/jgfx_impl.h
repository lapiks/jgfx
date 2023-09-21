#pragma once

#include "renderer_vk.h"
#include "jgfx/jgfx.h"

#include <memory>

namespace jgfx {
  struct ContextImpl {
    bool init(const InitInfo& initInfo);
    void shutdown();

    ShaderHandle newShader(const std::vector<char>& bytecode);
    ProgramHandle newProgram();

  private:
    std::unique_ptr<vk::RenderContextVK> vkCtx;

    HandleAllocator<ShaderHandle> shaderHandleAllocator;
  };
}
