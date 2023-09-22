#pragma once

#include "renderer_vk.h"
#include "jgfx/jgfx.h"

#include <memory>

namespace jgfx {
  struct ContextImpl {
    bool init(const InitInfo& initInfo);
    void shutdown();

    PipelineHandle newPipeline(ShaderHandle vertex, ShaderHandle fragment, PassHandle pass);
    PassHandle newPass();
    ShaderHandle newShader(const std::vector<char>& bytecode);
    ProgramHandle newProgram();

  private:
    std::unique_ptr<vk::RenderContextVK> vkCtx;

    HandleAllocator<PipelineHandle> pipelineHandleAlloc;
    HandleAllocator<PassHandle> passHandleAlloc;
    HandleAllocator<ShaderHandle> shaderHandleAlloc;
  };
}
