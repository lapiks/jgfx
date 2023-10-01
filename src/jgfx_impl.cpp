#pragma once
#include "jgfx_impl.h"

#include "jgfx/jgfx.h"

namespace jgfx {
  bool ContextImpl::init(const InitInfo& initInfo) {
    if (vkCtx) // already initialized
      return false;

    vkCtx = std::make_unique<vk::RenderContextVK>();

    _initInfo = initInfo;
    return vkCtx->init(initInfo);
  }

  void ContextImpl::shutdown() {
    vkCtx->shutdown();
  }

  void ContextImpl::reset(uint32_t width, uint32_t height) {
    _initInfo.resolution.width = width;
    _initInfo.resolution.height = height;
    _reset = true;
  }

  PipelineHandle ContextImpl::newPipeline(ShaderHandle vertex, ShaderHandle fragment, PassHandle pass) {
    PipelineHandle handle;
    pipelineHandleAlloc.allocate(handle);

    vkCtx->newPipeline(handle, vertex, fragment, pass);

    return handle;
  }

  PassHandle ContextImpl::newPass() {
    PassHandle handle;
    passHandleAlloc.allocate(handle);

    vkCtx->newPass(handle);

    return handle;
  }

  ShaderHandle ContextImpl::newShader(const std::vector<char>& bytecode) {
    ShaderHandle handle;
    shaderHandleAlloc.allocate(handle);

    vkCtx->newShader(handle, bytecode);

    return handle;
  }

  void ContextImpl::beginDefaultPass() {
    vkCtx->beginDefaultPass();
  }

  void ContextImpl::beginPass(PassHandle pass) {
    vkCtx->beginPass(pass);
  }

  void ContextImpl::applyPipeline(PipelineHandle pipe) {
    vkCtx->applyPipeline(pipe);
  }

  void ContextImpl::draw(uint32_t firstVertex, uint32_t vertexCount) {
    vkCtx->draw(firstVertex, vertexCount);
  }

  void ContextImpl::endPass() {
    vkCtx->endPass();
  }

  void ContextImpl::commitFrame() {
    if (_reset) {
      vkCtx->updateResolution(_initInfo.resolution);
      _reset = false;
    }
    vkCtx->commitFrame();
  }

}
