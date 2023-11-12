#pragma once

#include "renderer.h"
#include "jgfx/jgfx.h"

#include <memory>

constexpr int MAX_BUFFER_COMMANDS = 4 << 10;

namespace jgfx {
  enum CommandType {
    NewPipeline,
    NewPass,
    NewShader,
    NewProgram,
    NewBuffer,
    NewUniformBuffer,
    NewImage,
    BeginDefaultPass,
    BeginPass,
    ApplyPipeline,
    ApplyBindings,
    ApplyUniforms,
    Draw,
    DrawIndexed,
    EndPass,
    End,
  };

  struct CommandBuffer {
    uint8_t* _data;
    uint32_t _size = 0;
    uint32_t _currentPos = 0;

    CommandBuffer() {
      resize(1024);
    }

    ~CommandBuffer() {
      delete _data;
    }

    void resize(uint32_t size) {
      if (!_data) {
        _data = new uint8_t[size];
      }
      else {
        realloc(_data, size);
      }
      _size = size;
    }

    void write(const void* data, uint32_t size) {
      if (_currentPos + size > _size)
        resize(_currentPos + size);

      memcpy(&_data[_currentPos], data, size);
      _currentPos += size;
    }

    template<typename T>
    void write(const T& data) {
      size_t size = sizeof(data);
      write(reinterpret_cast<const uint8_t*>(&data), size);
    }

    void read(void* data, uint64_t size) {
      memcpy(data, &_data[_currentPos], size);
      _currentPos += size;
    }

    template<typename T>
    void read(T& data) {
      size_t size = sizeof(data);
      read(reinterpret_cast<uint8_t*>(&data), size);
    }

    void reset() {
      _currentPos = 0;
    }
  };

  template<typename T>
  struct HandleAllocator {
    uint16_t currentId = 0;
    inline void allocate(T& handle) {
      handle.id = currentId++;
    }
  };

  /// <summary>
  /// Private Implementation of the public context API
  /// </summary>
  struct ContextImpl {
    bool init(const InitInfo& initInfo);
    void shutdown();
    void reset(uint32_t width, uint32_t height);

    PipelineHandle newPipeline(const PipelineDesc& pipelineDesc);
    PassHandle newPass(const PassDesc& passDesc);
    ShaderHandle newShader(ShaderType type, const void* binData, uint32_t size);
    ProgramHandle newProgram(ShaderHandle vs, ShaderHandle fs);
    BufferHandle newBuffer(const void* data, uint32_t size, BufferType type);
    UniformBufferHandle newUniformBuffer(uint32_t size);
    ImageHandle newImage(const void* data, uint32_t size, const TextureDesc& desc);

    void beginDefaultPass();
    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void applyBindings(const Bindings& bindings);
    void applyUniforms(ShaderStage stage, const void* data, uint32_t size);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void drawIndexed(uint32_t firstIndex, uint32_t indexCount);
    void endPass();
    void commitFrame();

    CommandBuffer& startCommand(CommandType cmdType);
    void executeCommands();

  private:
    std::unique_ptr<RenderContext> _ctx;

    InitInfo _initInfo;
    bool _reset = false;

    CommandBuffer _cmdBuffer;

    HandleAllocator<PipelineHandle> pipelineHandleAlloc;
    HandleAllocator<PassHandle> passHandleAlloc;
    HandleAllocator<ShaderHandle> shaderHandleAlloc;
    HandleAllocator<ProgramHandle> programHandleAlloc;
    HandleAllocator<FramebufferHandle> framebufferHandleAlloc;
    HandleAllocator<BufferHandle> bufferHandleAlloc;
    HandleAllocator<UniformBufferHandle> uniformBufferHandleAlloc;
    HandleAllocator<ImageHandle> imageHandleAlloc;
  };
}
