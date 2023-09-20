#pragma once

#include <stdint.h>

namespace jgfx {
  struct PlatformData {
    void* nativeWindowHandle = nullptr;
  };

  struct CreateInfo {
    PlatformData platformData;
    uint32_t extensionCount = 0;
    const char** extensionNames = nullptr;
  };

  struct Context {
    bool init(const CreateInfo& init);
    void shutdown();
  };
}
