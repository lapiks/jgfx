#pragma once
#include "jgfx_impl.h"

#include "jgfx/jgfx.h"

namespace jgfx {
  bool ContextImpl::init(const InitInfo& initInfo) {
    return vkCtx.init(initInfo);
  }

  void ContextImpl::shutdown() {
    vkCtx.shutdown();
  }

  ShaderHandle ContextImpl::newShader() {
    return ShaderHandle();
  }

  ProgramHandle ContextImpl::newProgram() {
    return ProgramHandle();
  }

}
