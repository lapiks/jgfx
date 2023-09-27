#pragma once

#include <stdint.h>
#include <vector>

namespace jgfx {
  template<typename T>
  struct HandleAllocator {
    uint16_t currentId = 0;
    inline void allocate(T& handle) {
      handle.id = currentId++;
    }
  };
  
  constexpr uint16_t nullHandle = UINT16_MAX;

  #define JGFX_HANDLE(name) \
	struct name { uint16_t id = nullHandle; };

  struct PlatformData {
    void* nativeWindowHandle = nullptr;
  };

  struct Resolution {
    uint32_t width = 0;
    uint32_t height = 0;
  };

  struct InitInfo {
    PlatformData platformData;
    uint32_t extensionCount = 0;
    const char** extensionNames = nullptr;
    Resolution resolution;
  };

  JGFX_HANDLE(ShaderHandle)
  JGFX_HANDLE(PipelineHandle)
  JGFX_HANDLE(PassHandle)
  JGFX_HANDLE(FramebufferHandle)

  struct Context {
    // Initialization and shutdown
    bool init(const InitInfo& init);
    void shutdown();
    // Object creation
    PipelineHandle newPipeline(ShaderHandle vertex, ShaderHandle fragment, PassHandle pass);
    PassHandle newPass();
    ShaderHandle newShader(const std::vector<char>& binData);
    // Drawing
    void beginDefaultPass();
    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void endPass();
    void commitFrame();
  };
}
