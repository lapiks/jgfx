#pragma once

#include <vector>

#include "jgfx/jgfx.h"

namespace jgfx {
  constexpr int MAX_SHADERS = 512;
  constexpr int MAX_PROGRAMS = 512;
  constexpr int MAX_PIPELINES = 512;
  constexpr int MAX_PASSES = 512;
  constexpr int MAX_FRAMEBUFFERS = 512;
  constexpr int MAX_BUFFERS = 4 << 10;
  constexpr int MAX_IMAGES = 4 << 10;
  constexpr int MAX_FRAMES_IN_FLIGHT = 3;

  struct RenderContext {
    virtual bool init(const InitInfo& createInfo) = 0;
    virtual void shutdown() = 0;
    virtual void newShader(ShaderHandle handle, const void* binData, uint32_t size) = 0;
  };
}
