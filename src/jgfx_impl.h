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
    void reset(uint32_t width, uint32_t height);

    PipelineHandle newPipeline(ShaderHandle vertex, ShaderHandle fragment, PassHandle pass);
    PassHandle newPass();
    ShaderHandle newShader(const std::vector<char>& bytecode);

    void beginDefaultPass();
    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
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
  };
}
