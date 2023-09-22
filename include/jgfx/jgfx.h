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

  struct InitInfo {
    PlatformData platformData;
    uint32_t extensionCount = 0;
    const char** extensionNames = nullptr;
    uint32_t resolutionWidth = 0;
    uint32_t resolutionHeight = 0;
  };

  JGFX_HANDLE(ProgramHandle)
  JGFX_HANDLE(ShaderHandle)
  JGFX_HANDLE(PipelineHandle)

  struct Context {
    // Initialization and shutdown
    bool init(const InitInfo& init);
    void shutdown();
    // Object creation
    PipelineHandle newPipeline(ShaderHandle vertex, ShaderHandle fragment);
    ShaderHandle newShader(const std::vector<char>& binData);
    ProgramHandle newProgram(const std::vector<char>& binData);
    // 
  };
}
