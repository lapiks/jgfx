#pragma once

namespace jgfx {
  struct PlatformData {
    void* nativeWindowHandle = nullptr;
  };

  struct Init {
    PlatformData platformData;
  };

  struct Context {
    bool init(const Init& init);
    void shutdown();
  };
}
