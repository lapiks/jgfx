#pragma once

#include <vector>

#include "jgfx/jgfx.h"

namespace jgfx {
  struct RenderContext {
    virtual bool init(const InitInfo& createInfo) = 0;
    virtual void shutdown() = 0;
    virtual void newShader(ShaderHandle handle, const void* binData, uint32_t size) = 0;
  };
}
