#pragma once

#include "renderer_vk.h"

namespace jgfx {
  struct ContextImpl {
    bool init(const CreateInfo& createInfo);
    void shutdown();

  private:
    vk::RenderContextVK vkCtx;
  };
}
