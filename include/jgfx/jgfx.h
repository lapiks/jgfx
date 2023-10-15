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

  enum BufferType {
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
  };
  
  enum ShaderStage {
    VERTEX,
    FRAGMENT,
    ALL,
  };

  enum CullMode {
    NONE,
    FRONT,
    BACK,
  };

  JGFX_HANDLE(ShaderHandle)
  JGFX_HANDLE(PipelineHandle)
  JGFX_HANDLE(PassHandle)
  JGFX_HANDLE(FramebufferHandle)
  JGFX_HANDLE(BufferHandle)
  JGFX_HANDLE(UniformBufferHandle)
  JGFX_HANDLE(ImageHandle)

  struct Bindings {
    BufferHandle vertexBuffers[MAX_BUFFER_BIND];
    BufferHandle indexBuffer;
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

  struct PipelineDesc {
    ShaderHandle vs;
    ShaderHandle fs;
    VertexAttributes vertexAttributes;
    CullMode cullMode;
    PassHandle pass;
  };

  struct PassDesc {

  };

  struct Context {
    // Initialization and shutdown
    bool init(const InitInfo& init);
    void shutdown();
    void reset(uint32_t width, uint32_t height);
    // Object creation
    PipelineHandle newPipeline(const PipelineDesc& pipelineDesc);
    PassHandle newPass(const PassDesc& passDesc);
    ShaderHandle newShader(const void* binData, uint32_t size);
    BufferHandle newBuffer(const void* data, uint32_t size, BufferType type);
    ImageHandle newImage();
    // Drawing
    void beginDefaultPass();
    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void applyBindings(const Bindings& bindings);
    void applyUniforms(ShaderStage stage, const void* data, uint32_t size);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void drawIndexed(uint32_t firstIndex, uint32_t indexCount);
    void endPass();
    void commitFrame();
  };
}
