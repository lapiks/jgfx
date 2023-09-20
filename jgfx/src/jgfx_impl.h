#pragma once

#include "renderer_vk.h"

namespace jgfx {
  struct ContextImpl {
    bool init(const InitInfo& initInfo);
    void shutdown();

  private:
    vk::RenderContextVK vkCtx;
  };
}
