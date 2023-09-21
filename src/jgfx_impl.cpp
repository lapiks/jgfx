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

  ShaderHandle ContextImpl::newShader(const std::vector<char>& bytecode) {
    ShaderHandle handle;
    allocate(handle, shaderHandleAllocator);

    vkCtx->newShader(handle, bytecode);

    return handle;
  }

  ProgramHandle ContextImpl::newProgram() {
    return ProgramHandle();
  }

}
