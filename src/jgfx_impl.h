#pragma once

#include "renderer_vk.h"
#include "jgfx/jgfx.h"

#include <memory>

namespace jgfx {
  /// <summary>
  /// Private Implementation of the public context api
  /// </summary>
  struct ContextImpl {
    bool init(const InitInfo& initInfo);
    void shutdown();

    PipelineHandle newPipeline(ShaderHandle vertex, ShaderHandle fragment, PassHandle pass);
    PassHandle newPass();
    ShaderHandle newShader(const std::vector<char>& bytecode);

    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void endPass();

  private:
    std::unique_ptr<vk::RenderContextVK> vkCtx;

    HandleAllocator<PipelineHandle> pipelineHandleAlloc;
    HandleAllocator<PassHandle> passHandleAlloc;
    HandleAllocator<ShaderHandle> shaderHandleAlloc;
    HandleAllocator<FramebufferHandle> framebufferHandleAlloc;
  };
}
