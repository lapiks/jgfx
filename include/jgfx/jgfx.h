#pragma once

#include <stdint.h>
#include <vector>

namespace jgfx {
  constexpr uint16_t MAX_BUFFER_BIND = 8;
  constexpr uint16_t MAX_VERTEX_ATTRIBUTES = 16;
  
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
    std::vector<const char*> extensionNames;
    Resolution resolution;
  };

  enum AttribType {
    UNKNOWN,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
  };

  JGFX_HANDLE(ShaderHandle)
  JGFX_HANDLE(PipelineHandle)
  JGFX_HANDLE(PassHandle)
  JGFX_HANDLE(FramebufferHandle)
  JGFX_HANDLE(BufferHandle)

  struct Bindings {
    BufferHandle vertexBuffers[MAX_BUFFER_BIND];
  };

  struct VertexAttributes {
    void begin();
    void add(uint32_t location, AttribType type);
    void end();

    uint32_t _offsets[MAX_VERTEX_ATTRIBUTES];
    AttribType _types[MAX_VERTEX_ATTRIBUTES];
    uint32_t _stride = 0;
    uint16_t _attrCount = 0;
  };

  struct Context {
    // Initialization and shutdown
    bool init(const InitInfo& init);
    void shutdown();
    void reset(uint32_t width, uint32_t height);
    // Object creation
    PipelineHandle newPipeline(ShaderHandle vertex, ShaderHandle fragment, PassHandle pass, VertexAttributes attr);
    PassHandle newPass();
    ShaderHandle newShader(const std::vector<char>& binData);
    BufferHandle newBuffer(const void* data, uint32_t size);
    // Drawing
    void beginDefaultPass();
    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void applyBindings(const Bindings& bindings);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void endPass();
    void commitFrame();
  };
}
