#pragma once
#include "jgfx_impl.h"

#include "jgfx/jgfx.h"

namespace jgfx {
  bool ContextImpl::init(const InitInfo& initInfo) {
    if (vkCtx) // already initialized
      return false;

    vkCtx = std::make_unique<vk::RenderContextVK>();

    return vkCtx->init(initInfo);
  }

  void ContextImpl::shutdown() {
    vkCtx->shutdown();
  }

  PipelineHandle ContextImpl::newPipeline(ShaderHandle vertex, ShaderHandle fragment) {
    PipelineHandle handle;
    pipelineHandleAlloc.allocate(handle);

    vkCtx->newPipeline(handle, vertex, fragment);

    return handle;
  }

  ShaderHandle ContextImpl::newShader(const std::vector<char>& bytecode) {
    ShaderHandle handle;
    shaderHandleAlloc.allocate(handle);

    vkCtx->newShader(handle, bytecode);

    return handle;
  }

  ProgramHandle ContextImpl::newProgram() {
    return ProgramHandle();
  }

}
