#pragma once

#include <stdint.h>

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

  struct Context {
    bool init(const InitInfo& init);
    void shutdown();
  };
}
