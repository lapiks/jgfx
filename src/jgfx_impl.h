#pragma once

#include "renderer_vk.h"
#include "jgfx/jgfx.h"

#include <memory>

namespace jgfx {
  template<typename T>
  struct HandleAllocator {
    uint16_t currentId = 0;
    inline void allocate(T& handle) {
      handle.id = currentId++;
    }
  };

  /// <summary>
  /// Private Implementation of the public context api
  /// </summary>
  struct ContextImpl {
    bool init(const InitInfo& initInfo);
    void shutdown();
    void reset(uint32_t width, uint32_t height);

    PipelineHandle newPipeline(const PipelineDesc& pipelineDesc);
    PassHandle newPass(const PassDesc& passDesc);
    ShaderHandle newShader(const void* binData, uint32_t size);
    BufferHandle newBuffer(const void* data, uint32_t size, BufferType type);
    UniformBufferHandle newUniformBuffer(uint32_t size);
    ImageHandle newImage();

    void beginDefaultPass();
    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void applyBindings(const Bindings& bindings);
    void applyUniforms(ShaderStage stage, const void* data, uint32_t size);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void drawIndexed(uint32_t firstIndex, uint32_t indexCount);
    void endPass();
    void commitFrame();

  private:
    std::unique_ptr<vk::RenderContextVK> vkCtx;

    InitInfo _initInfo;
    bool _reset = false;

    HandleAllocator<PipelineHandle> pipelineHandleAlloc;
    HandleAllocator<PassHandle> passHandleAlloc;
    HandleAllocator<ShaderHandle> shaderHandleAlloc;
    HandleAllocator<FramebufferHandle> framebufferHandleAlloc;
    HandleAllocator<BufferHandle> bufferHandleAlloc;
    HandleAllocator<UniformBufferHandle> uniformBufferHandleAlloc;
    HandleAllocator<ImageHandle> imageHandleAlloc;
  };
}
