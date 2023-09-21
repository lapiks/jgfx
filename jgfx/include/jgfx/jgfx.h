#pragma once

#include <stdint.h>
#include <vector>

namespace jgfx 
{
  #define JGFX_HANDLE(name) \
	struct name { uint16_t id; };

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

  struct Context {
    // Initialization and shutdown
    bool init(const InitInfo& init);
    void shutdown();
    // Object creation
    ShaderHandle newShader(const std::vector<char>& binData);
    ProgramHandle newProgram(const std::vector<char>& binData);
    // 
  };
}
