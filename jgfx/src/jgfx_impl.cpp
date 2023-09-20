#pragma once
#include "jgfx_impl.h"

#include "jgfx/jgfx.h"

namespace jgfx {
  bool ContextImpl::init(const CreateInfo& createInfo)
  {
    return vkCtx.init(createInfo);
  }

  void ContextImpl::shutdown()
  {
    vkCtx.shutdown();
  }
}
